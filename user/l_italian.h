// ITALIAN text

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
	{"INVIO", K_ENTER},
	{"ESC", K_ESCAPE},
	{"SPAZIO", K_SPACE},
	{"INDIETRO", K_BACKSPACE},
	{"SU", K_UPARROW},
	{"GIÙ", K_DOWNARROW},
	{"SINISTRA", K_LEFTARROW},
	{"DESTRA", K_RIGHTARROW},
	
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
	{"CANC", K_DEL},
	{"PGGIÙ", K_PGDN},
	{"PGSU", K_PGUP},
	{"HOME", K_HOME},
	{"FINE", K_END},
	
	{"MOUSE1", K_MOUSE1},
	{"MOUSE2", K_MOUSE2},
	{"MOUSE3", K_MOUSE3},
	
	{"JOY1", K_JOY1},
	{"JOY2", K_JOY2},
	{"JOY3", K_JOY3},
	{"JOY4", K_JOY4},
	
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
	
	{"TN_HOME",			K_KP_HOME },
	{"TN_SU",		K_KP_UPARROW },
	{"TN_PGSU",			K_KP_PGUP },
	{"TN_SINISTRA",	K_KP_LEFTARROW },
	{"TN_5",			K_KP_5 },
	{"TN_DESTRA",	K_KP_RIGHTARROW },
	{"TN_FINE",			K_KP_END },
	{"TN_GIÙ",	K_KP_DOWNARROW },
	{"TN_PGGIÙ",			K_KP_PGDN },
	{"TN_INVIO",		K_KP_ENTER },
	{"TN_INS",			K_KP_INS },
	{"TN_CANC",			K_KP_DEL },
	{"TN_SLASH",		K_KP_SLASH },
	{"TN_MENO",		K_KP_MINUS },
	{"TN_PIÙ",			K_KP_PLUS },
	
	{"ROTMOUSESU", K_MWHEELUP },
	{"ROTMOUSEGIÙ", K_MWHEELDOWN },
	
	{"PAUSA", K_PAUSE},
	
	{"PUNTO E VIRGOLA", ';'},	// because a raw semicolon seperates commands
	
	{NULL,0}
};

// vkey conversion table
//==========================================================================
static byte        scantokey[128] = 
                    { 
//  0           1       2       3       4       5       6       7 
//  8           9       A       B       C       D       E       F 
    0  ,	27,     '1',    '2',    '3',    '4',    '5',    '6', 
    '7',    '8',    '9',    '0',    '\'',    'ì',    K_BACKSPACE, 9,  // 0
    'q',    'w',    'e',    'r',    't',    'y',    'u',    'i', 
    'o',    'p',    'è',    '+',    13 ,    K_CTRL,'a',  's',      // 1 
    'd',    'f',    'g',    'h',    'j',    'k',    'l',    'ò', 
    'à' ,    '`',    K_SHIFT,'<',  'z',    'x',    'c',    'v',      // 2 
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
	"Hai raccolto",                            // weapon pickup TX NOTE:+ the items in the weapons section below.
	"Hai raccolto",			                // health pickup TX NOTE:+ one of the 3 health items below (health kit, Goldenorb, Wraith orb.
	"Hai raccolto",						    // armor good pickup TX NOTE:+ the items in the armor section below.
	"Hai raccolto",						    // boost pickup TX NOTE:not used - DON'T TRANSLATE.
	
	// **************************************************************
	"Ottenuta gemma di salvataggio!",            // pick up 'save gem'
	// **************************************************************
	
	
	"POTENZA",                                    // use power boost TX NOTE:must match statusbar.xls .
	"ATTACCO",                                   // use attack boost TX NOTE:must match statusbar.xls .
	"VELOCITÀ",                                    // use speed boost TX NOTE:must match statusbar.xls .
	"BALZO",                                     // use jump boost TX NOTE:must match statusbar.xls .
	"VITALITÀ",                                 // use health boost TX NOTE:must match statusbar.xls .
	
	"È ora di andarsene",                           // bomb explode TX NOTE:"let's get out of here!.
	"Ti serve una bottiglia per la miscelazione",    // need bottle to mix ingredients TX NOTE:sentence.
	"Elementi richiesti: ",                          // ingredient required TX NOTE:+ bomb item, short desc below.
	"Hai creato un esplosivo...",                  // bomb created TX NOTE:sentence.
	"Hai trovato ",                                // bomb found, TX NOTE:+ one of the ingredients or a bottle.
	"un sacchetto di zolfo",						// bomb ingredient 1
	"un pezzo di carbone",                           // bomb ingredient 2
	"una fiala di salnitro",                      // bomb ingredient 3
	"una bottiglia vuota",                          // bomb bottle
	
	"zolfo",                                  // bomb item, short description
	"carbone",                                 // bomb item, short description
	"salnitro",                                // bomb item, short description
	
	"Hai letto ",                            // read book TX NOTE:+ one of two books below.
	"Non puoi farne uso qui.",                // no read book TX NOTE:sentence.
	"il Libro degli incantesimi di Wyndrax",                    // Wyndrax spellbook
//	"il Libro satanico del Male",                 // Satanic book of evil
	
	"Hai raccolto",                            // key pickup TX NOTE:one of the 3 following items.
	"la chiave della cripta",                            // crypt key name
	"la chiave di Wyndrax",                          // Wyndrax key name
	"la chiave di una cella",                    // Prison cell key
	
	"la scheda di controllo rosso",						// SCG[11/15/99]: red control card name
	"la scheda di controllo blu",					// SCG[11/6/99]: blue control card name
	"la scheda di controllo verde",					// SCG[11/6/99]: green control card name
	"la scheda di controllo giallo",					// SCG[11/6/99]: yellow control card name
	"l'Esagono",							// SCG[11/6/99]: hex keystone name
	"il Quadrilatero",						// SCG[11/6/99]: quad keystone name
	"il Trigono",						// SCG[11/6/99]: trigon keystone name
	"il Megascudo",							// SCG[11/14/99]: Megashield name
	"il teschio del mana",							// SCG[11/14/99]: 
	"l'antidoto al veleno",						// SCG[11/14/99]: 
	"la dracma",								// SCG[11/14/99]: 
	"il Corno di Caronte",						// SCG[11/14/99]: 
	"un pezzo della chiave AEGIS (A)",				// SCG[11/14/99]: A of AEGIS key name
	"un pezzo della chiave AEGIS (E)",				// SCG[11/14/99]: E of AEGIS key name
	"un pezzo della chiave AEGIS (G)",				// SCG[11/14/99]: G of AEGIS key name
	"un pezzo della chiave AEGIS (I)",				// SCG[11/14/99]: I of AEGIS key name
	"un pezzo della chiave AEGIS (S)",				// SCG[11/14/99]: S of AEGIS key name
	"un frammento del Purificatore",					// SCG[11/14/99]: 
	"la cassa nera",							// SCG[11/14/99]: 
	"l'Anello della Protezione dal Fuoco",				// SCG[11/14/99]: 
	"l'Anello della Protezione dai Non-morti",			// SCG[11/14/99]: 
	"l'Anello della Protezione dai Fulmini",			// SCG[11/14/99]: 
	"la tuta anti-ambiente",							// SCG[11/14/99]: 
	
	"il kit medico",                           // health kit
	"l'anima d'oro",                           // Goldensoul
	"la sfera dello spettro",							// wraith orb
	
	// *******************************************************
	"una gemma di salvataggio",                  // 'save gem'
	"Non hai partite salvate",					//cek[12-13-99]
	"la fiala della salute",							// cek[1-3-00]
	// *******************************************************

	// cek[2-2-00]: altered the following ***********************************************
	"il pacchetto della salute",
	"il vaso della salute",
	"il fiasco della salute",
	"la cesta della salute",
	"la scatola della salute",
	"L'antidoto è esaurito.",

	// SCG[2/8/00]: messages for completed artifacts
	"Hai trovato tutte le rune AEGIS!",
	"Hai trovato le tre chiavi!",
	"Hai assemblato il Purificatore!"
};



// menu-related TX NOTE:all independent words -- no grammatical inflection.
static char *tongue_menu[] =
{
	"Avanti",                                  // key bind to Move Forward
	"Indietro",                                 // key bind to Move Backward
	"Sinistra",                                // key bind to Step Left
	"Destra",                               // key bind to Step Right
	"Attacco",                                   // key bind to Attack
	"Salto",                                     // key bind to Jump/Up
	"Usa/Attiva",                              // key bind to Use/Operate object
	"Abbassati",                                   // key bind to Crouch/Down
	"Arma seg.",                              // key bind to Next Weapon
	"Arma prec.",                              // key bind to Prev Weapon
	"Gira a sinistra",                                // key bind to Turn Left
	"Gira a destra",                               // key bind to Turn Right
	"Corri",                                      // key bind to Run
	"Spostamento lat.",                             // key bind to  Sidestep
	"Guarda in su",                                  // key bind to Look Up
	"Guarda in giù",                                // key bind to Look Down
	"Centra visuale",                              // key bind to Center View
	"Sguardo/mouse",                               // key bind to Mouse Look
	"Sguardo/tastiera",                         // key bind to Keyboard Look
	"Aumenta visuale",                             // key bind to Screen size up
	"Riduci visuale",                           // key bind to Screen size down
	"Mostra tutti HUD",                            // key bind to Show All Huds
	"Apri inv.",                                // key bind to Inventory Open
	"Usa oggetto inv.",                            // key bind to Use Item
	//  "Abbandona oggetto",                                // key bind to Drop Item
	"Oggetto seg.",                                // key bind to Next Item
	"Oggetto prec.",                                // key bind to Prev Item
	"Cambia compagno",                            // key bind to Swap Sidekick
	"Comando seg.",                             // key bind to Command Next
	"Comando prec.",                             // key bind to Command Prev
	"Applica comando",                            // key bind to Command Apply
	
	ARROW_LEFT_STRING  " Sinistra",                 // left arrow key
	ARROW_UP_STRING    " Su",                   // up arrow key
	ARROW_DOWN_STRING  " Giù",                 // down arrow key
	ARROW_RIGHT_STRING " Destra",                // right arrow key
	"--",                                       // no bind setting for this key
	
	"Clicca o INVIO per impostare, CANC per eliminare",     // set/delete key TX NOTE:sentence.
	"Clicca o INVIO per selezionare",                 // select key TX NOTE:sentence.
	"Premi il tasto per impostare, ESC per annullare",             // set/abort TX NOTE:sentence.
	
	"Predefiniti",                                 // default button
	
	"Nessuno",                                     // no mouse bindings
	"Movimento sinistra/destra",                          // mouse "move left and right" setting
	"Giro sinistra/destra",                          // mouse "turn left and right" setting
	"Guarda su/giù",                             // mouse "look up and down" setting
	"Movimento avanti/indietro",                           // mouse "move forward and back" setting
	"Inverti asse Y",                           // reverse mouse axis
	"Sguardo col mouse",                               // Mouse-Look
	"Sguardo laterale",                              // Mouse-Look
	"Asse X",                                   // x axis
	"Asse Y ",                                   // y axis
	"Pulsante 1",                                 // button 1 label
	"Pulsante 2",                                 // button 2 label
	"Pulsante 3",                                 // button 3 label
	"Seleziona un comando, ESC per annullare",          // select a command, ESCape to cancel
	"Gira Sx/Dx",                                 // turn left and right, abbreviated
	"Spostamento Sx/Dx",                               // strafe left and right, abbreviated
	"Guarda S/G",                                 // look up and down, abbreviated
	"Movimento A/I",                                 // move forward and back, abbreviated
	"Sensibilità X",                            // mouse sensitivity for x-axis
	"Sensibilità Y",                            // mouse sensitivity for y-axis
	"Configura opzioni mouse:",                 // configure mouse options  TX NOTE:sentence.
	
	"Sfida mortale",                               // 'deathmatch' game
	"Tocco mortale",                                 // 'deathtag' game
	"Cooperativa",                              // 'cooperative' game
	
	"Facile",                                     // easy difficulty
	"Media",                                   // medium difficulty
	"Difficile",                                     // hard difficulty
	
	// ******************************************
	"Trova partita su Internet",					// SCG[12/1/99]: 
	"Trova partita locale",						// SCG[12/1/99]: 
	"Comincia partita Multigioc",					// SCG[12/1/99]: 

	"LAN",                                      // local area network game
	"Internet",                                 // internet game
	"Unisciti",                                     // join server
	"Ospita",                                      // host new  server
	
	
	"Aggiungi" ARROW_DOWN_STRING,                   // add address
	"Aggiorna",                                  // refresh network games
	"Impostazione",                                    // player setup
	
	//"Handedness",                               // handedness label TX NOTE:not used. DO NOT TRANSLATE..
	//"Left",                                     // handedness, left TX NOTE:not used. DO NOT TRANSLATE..
	//"Ambi-",                                    // handedness, center TX NOTE:ambi-dextrous. TX NOTE:not used. DO NOT TRANSLATE..
	//"Right",                                    // handedness, right TX NOTE:not used. DO NOT TRANSLATE..
	
	"Velocità connessione",                             // connection speed label
	"28.8k",                                    // connection type 1
	"56.6k",                                    // connection type 2
	"ISDN",                                 // connection type 3
	"Cavo/DSL",                                // connection type 4
	"T1/LAN",                                   // connection type 5
	"Definita dall'utente",                             // connection type 6 TX NOTE:user enters his own connection type.
	
	// ******************************************
	"Tempo",								// SCG[12/2/99]: 
	"Colore personaggio",						// SCG[12/2/99]: 
	"Espisodio iniziale",							// SCG[12/2/99]: 
	"Scelta squadra",								// SCG[12/1/99]: 
	"Bandiere",								// SCG[12/1/99]: 
	"Colore squadre 1",								// SCG[12/1/99]: 
	"Colore squadre 2",								// SCG[12/1/99]: 
	"Punteggio limite",								// SCG[12/1/99]: 
	// ******************************************

	"Uccisioni",								// SCG[12/1/99]: 

	// ******************************************
	"Livello limite",                              
	"Espisodio iniziale",							// SCG[12/2/99]: 
	"Mappa del gioco",									// SCG[12/1/99]: 
	// ******************************************

	"Modalità partita",                                // game mode (deathmatch,deathtag,co-op)
	"Livello abilità",                              // skill level
	"Tempo",                               // time limit
	"Max giocatori",                              // maximum players
	"Personaggio",                                // character
	"Colore base",                               // base color
	"Via!",                                   // start multiplayer game
	
	"Permanenza arma",                             // weapons remain with you into the next area of play
	"Rigenera oggetti",                            // items rematerialize on the map
	"Genera lontano",                             // place user's character at the farthest point from other players
	"Consenti uscita",                            // allow players to exit the level wihtout being killed first
	"Stessa mappa",                                 // use the same map as last time
	"Rigener. forzata",                            // make the respawn command happen now.
	"Danni per cadute",                           // players take damage from falling down
	"Powerup istant.",                            // make instant power-ups available in the game
	"Ammetti powerup",                             // allow power-ups to be used in the game
	"Ammetti salute",                             // allow health kits to be used in the game
	"Ammetti corazze",                              // allow armor to be used in the game
	"Munizioni infinite",                            // unlimited ammunition
	"Campo visivo",                                // limits the user's field-of-vision
	"Gioco a squadre",                                 // team-play
	"Danni/compagni",                           // hurt teammates allows players to cause damage to team-mates.
	"Cambio rapido",                              // allows a player to change weapons faster than normal
	"Passi",                                // footsteps become audible or silent
	"Ammetti uncino",                               // allow hook
	"Proiettili",                           // show shotgun shells?
	
	"Indirizzo IP server",                        // server internet provider address
	"Nessuna partita locale",                     // no local games found TX NOTE:sentence.
	"Mappa iniziale",                             // starting map
	"Nome server",                              // server name
	"Nome giocatore",                              // player name
	"Nome squadra",                                // team name
	
	"Vuoi fare Harakiri?",                        // confirm quit TX NOTE:Do not translate Harakiri--it's Jpn.
	
	"Sì",                             // 'yes' response
	"No",                             // 'no' response

	"Salva partita",								// cek[1-3-00] changed from 'save' to 'save game'
	"Carica Config.",								//cek[12-6-99]
	"Salva Config.",								//cek[12-6-99]
	"Elimina",							//cek[12-6-99]
	"Seleziona file di configurazione",				//cek[12-6-99]
	"Cancella file?",						//cek[12-6-99]
												
	"Carica gioco",								//cek[12-6-99]
	"Armat.",									//cek[12-6-99]
	"Salute",									//cek[12-6-99]
	"Livello",									//cek[12-6-99]
	"Mostri",									//cek[12-6-99]
	"Segreti",									//cek[12-6-99]
	"Tempo",										//cek[12-6-99]
	"Totale",									//cek[12-6-99]
	"Episodio",									//cek[12-6-99]

	"Usa sonoro 3D",								//cek[12-6-99]

	"salvato",									//cek[12-7-99]
	"caricato",									//cek[12-7-99]
	"Eliminata",									//cek[12-7-99]

	"Fluidità mouse",								//cek[12-7-99]
	"Rotella su",									//cek[12-7-99]
	"Rotella giù",								//cek[12-7-99]

	"No ALT-TAB",						//cek[12-7-99]
	"Sangue e carneficina",						//cek[12-7-99]
	"Oscillazione arma",							//cek[12-7-99]
	"Cambio arma autom.",						//cek[12-7-99]

	"Intensità",								//cek[12-7-99] cek[12-13-99]
	"Riflesso armi",							//cek[12-7-99]
	"Buchi proiettili",								// SCG[1/4/00]: 

	"Avanti",									//cek[12-9-99]
	"Laterale",										//cek[12-9-99]
	"Su",										//cek[12-9-99]
	"Campo",									//cek[12-9-99]
	"Deviato",										//cek[12-9-99]
	"Usa il joystick",								//cek[12-9-99]
	"Configura assi",							//cek[12-9-99]
	"Configura joystick",						//cek[12-9-99]
	"Seleziona azione per questo pulsante",			//cek[12-9-99]
	"Premi il pulsante del joystick da configurare",
	"Soglia",								//cek[12-9-99]
	"Sensibilità",								//cek[12-9-99]

	"Cattura la bandiera",							//cek[12-10-99]
	"Indietro",									//cek[12-10-99]
	"56.6k",									//cek[12-10-99]
	"Auto",										//cek[12-10-99]
	"Team 1",									//cek[12-10-99]
	"Team 2",									//cek[12-10-99]

	"Esegui introduzione",								//cek[12-13-99]
	"Utilizza abilità",							//cek[12-13-99]
	"Mouse velocità",								//cek[12-13-99]

	"\"Comp. Prendi\"",								// cek[1-3-00]
	"\"Comp. Vieni\"",							// cek[1-3-00]
	"\"Comp. Attacca\"",							// cek[1-3-00]
	"\"Comp. Ritirati\"",							// cek[1-3-00]
	"\"Comp. Fermati\"",							// cek[1-3-00]

	"Seleziona arma %d",							// cek[1-4-00]  as in "Select Weapon 1" (leave the %d)
	"Scorri HUD",								// cek[1-5-00]  -- for these, translate HUD to something short and reasonably
	"Prossimo HUD",									// cek[1-5-00]  -- similar in meaning.
	"HUD preced.",									// cek[1-5-00]
	"Usa HUD",									// cek[1-5-00]
	"Parla",										// cek[1-11-00]
	"Punteggi",								// SCG[1/13/00]: 
	"Parla al gruppo",								// cek[1-22-00]
	"%s è già assengnato a %s.",				// cek[1-22-00]  as in "F is already assigned to attack" (leave in %s)
	"Sostituire?",									// cek[1-22-00]

	"Vai alla lobby",								// cek[2-1-00]
	"Comunità DK",								// cek[2-1-00]
	"Continua?",								// cek[2-1-00]

	// for the next two, all \n in the strings denotes a string break.  These must remain in place and the translated
	// string should have them in roughly the same place.
	"Continuando uscirai da Daikatana\ne lancerai il browser Mplayer.",	// cek[2-1-00]
	"Per le ultime notizie, le chat, i forum e le\nclassifiche visita www.daikatananews.com.\nContinuando lancerai il tuo browser.",			// cek[2-1-00]

	"Arma %d",									// cek[3-6-00]: treat as "weapon 1"
	"Annullano",									// cek[3-9-00]
	"www.planetdaikatana.com",													// cek[3-9-00]
	"Vai alla www.planetdaikatana.com.\nContinuando lancerai il tuo browser.",			// cek[3-9-00]
};


static char *tongue_menu_options[] =
{
	"Corsa continua",                               // makes user's characters run everywhere
	"Visuale centrata",                               // look + jump
	"Visuale laterale",                               // look + strafe (shoot wildly)
	"Puntamento automatico",                           // computer aims at nearest target for the user
	"Mirino",                                // a crosshair is displayed
	"Arma visibile",                           // weapon is shown
	"Sottotitoli",                                // subtitles
	
	"Volume effetti sonori",                      // volume for sound fx
	"Volume musica",                             // volume for music
	"Volume film",                         // volume for cinematics
	
	"Software",                                 // video mode, software
	"OpenGL",                                   // video mode, opengl
	"Applica modifiche",                            // apply video mode changes TX NOTE:sentence.
	
	"Pieno schermo",                               // make view use the full screen
// SCG[1/4/00]: 	"Stipple Alpha",                            // stipple alpha TX NOTE:this is a type of speed optimization technique. Don't translate.
	"Ombre",                                  // allow shadows
	"Driver",                                   // video driver
	"Risoluzione",                               // choose/set screen display resolution
	"Dimensioni schermo",                              // choose/set screen size
	"Luminosità",                               // choose/set brightness
	"Qualità texture",                          // choose/set quality of 3D image textures
	"Visibilità barra di stato",                     // choose/set whether the status bar is visible.
	//**************************************************
	"Dettaglio nebbia/acqua",							// SCG[12/4/99]: 
	
	"Ronin",									// difficulty level easy
	"Samurai",									// difficulty level medium
	"Shogun",									// difficulty level hard
	"Neve e pioggia",							// cek[1-11-00]
	"Nebbia"										// cek[1-11-00]		
	//**************************************************
};



//descriptions for the texture quality settings.
static char *texture_quality_desc[] =
{
	"8/8",
	"4/8",
	"2/8",
	"1/8"
};


// weapon names TX NOTE:"you picked up" +.TX NOTE:translate what you can. "localize" spellings of anything else.
static char *tongue_weapons[] =
{
	"No munizioni",		//only shows up if you have no ammo for the weapon you're trying to use. TX NOTE:treat like sentence "you have "no ammo".
	"la balestra",
	"le frecce della balestra",
	"il lanciadardi",
	"i dardi",
	"il Vitzatergo al C4",
	"i globi di C4",
// SCG[1/4/00]: 	"Celestril",
// SCG[1/4/00]: 	"la barriera di Celestril",
	"la Daikatana",
// SCG[1/4/00]: 	"l'Esilio della Daikatana",
// SCG[1/4/00]: 	"la Barriera della Daikatana",
// SCG[1/4/00]: 	"la Lama Danzante della Daikatana",
// SCG[1/4/00]: 	"la Mischia della Daikatana",
// SCG[1/4/00]: 	"il Possesso della Daikatana",
// SCG[1/4/00]: 	"il Proteo della Daikatana",
	"il disco di Dedalo",
	"il disco",
	"il guanto frantumatore",
// SCG[1/4/00]: 	"la torcia norvegese",
	"l'Automark 2020", // TX NOTE:a ficticious automatic pistol--US game currently reads "Glock", but that's a trademark.
	"i proiettili",
	"il maglio",
	"il disintegratore a ioni",
	"la sfera di ioni",
	"il kineticore",
	"la palla d'acciaio",
	"il metamaser",
	"le munizioni del metamaser",
// SCG[1/4/00]: 	"la mano d'oro di Mida",
// SCG[1/4/00]: 	"il pezzo di pirite",
	"l'incubo di Nharre",
	"alcuni brandelli",					// SCG[11/28/99]: 
	"il novabeam",
	"una novacell",					// SCG[11/28/99]: 
	"il ripgun",
	"le granate di cordite",
	"l'onda d'urto",
	"la sfera dello shock",
	"il fucile semiautomatico",
	"le cartucce",
	"il sidewinder",
	"i missili sidewinder",			// SCG[11/28/99]: 
	"l'artiglio d'argento",		
	"il lanciagranate",
	"le granate",
	"lo scettro di Stavros",
	"le rocce di lava",
	"il bagliore solare",
	"il bagliore solare",
// SCG[1/4/00]: 	"l'arpione elettrico",
	"il tridente di Poseidone",
	"le punte del tridente",
	"il bastone velenoso",
	"la sfera dello shock",					// SCG[1/4/00]: changed in english version
	"il ciuffo di Wyndrax",
	"i ciuffi",
	"L'occhio di Zeus",
	"il dardo",
	"le mani di gas definitive"
};

// armor TX NOTE:"you picked up" +.TX NOTE:translate what you can. "localize" spellings of anything else.
static char *tongue_armors[] =
{
	"la corazza in plasteel",
	"la corazza cromata",
	"la corazza d'argento",
	"la corazza d'oro",
	"la corazza di maglia",
	"la corazza di diamante nero",
	"la corazza di Kevlar",
	"la corazza d'ebanite"
};


// monster names TX NOTE:NOMINITIVE CASE. translate what you can. "localize" spellings of anything else.
static char *tongue_monsters[] =
{
	"Il cinghiale da battaglia robotizzato",
	"Il prigioniero nero",
	"Il buboide",         // TX NOTE:must match script.
	"La telecamera robotizzata",
	"Il centurione",
	"The Cerberus",         // TX NOTE:deleted. don't translate.
	"Il mitragliere volante",
	"La cariatide",
	"Il coccodrillo",
// SCG[1/4/00]: 	"I ciclopi",
	"La sfera della morte robotizzata",
	"Il pipistrello dell'Apocalisse",
	"Il dragone",         // TX NOTE:must match script.
	"Lo gnomo",
	"La gang di donne",
	"Il traghettatore",
	"L'arciere",
	"La rana robotizzata",
	"Re Garroth",
	"Il pesce d'oro",
	"Il grifone",
	"L'arpia",
	"Il carceriere",
	"Kage",
	"King Minos",         // TX NOTE:deleted. don't translate.
	"Il primo cavaliere",
	"Il secondo cavaliere",
	"La scimmia da laboratorio",
	"L'uomo del laboratorio",
	"La pistola laser robotizzata",
	"Il lupo mannaro",
	"Medusa",         // TX NOTE:must match script.
	"Il minotauro",
	"La guardia di Mishima",         // TX NOTE:must match script.
	"Nharre",              // TX NOTE:must match script  don't translate
	"Il topo della peste",
	"Il topo di fogna",
	"Il sacerdote",
	"Il prigioniero",
	"Il proto-bozzolo robotizzato",
	"Il cervello artigliato",
	"Il RageMaster 5000 robotizzato",
	"Il membro della gang coi razzi",
	"Il poliziotto militare coi razzi",
	"Il verme putrefatto",
	"Il satiro",
	"Il commando della Marina",         // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
// SCG[1/4/00]: 	"Il sommozzatore della Marina",            // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"Il capitano della Marina",         // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"La ragazza della Marina",         // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"Lo squalo",
	"La zanzara assassina",
	"Lo scheletro",
	"Il lavoratore magro",
	"Il lavoratore grasso",
	"Il servo di fango robotizzato",
	"Il piccolo ragno",
	"Il ragno",
	"Stavros",         // TX NOTE:must match script.
	"Il chirurgo",
	"Il ladro",
	"La zanzara del tuono",
// SCG[1/4/00]: 	"Il segugio robotizzato",
	"Il membro della gang con l'Uzi",
	"Il prigioniero bianco",
	"Lo stregone",         // TX NOTE:must match script.
	"Wyndrax",         // TX NOTE:must match script.
	"Gabbiano",
	"Lucciola",
	"Spiritello",
	"Mikiko"									// SCG[12/5/99]: 
};

// monster names ACCUSATIVE CASE. NOT USED IN ENGLISH. translate what you can. "localize" spellings of anything else.
static char *tongue_monsters_acc[] =
{
	"il cinghiale da battaglia robotizzato",
	"il prigioniero nero",
	"il buboide",         // TX NOTE:must match script.
	"la telecamera robotizzata",
	"il centurione",
	"the Cerberus",         // TX NOTE:deleted. don't translate.
	"il mitragliere volante",
	"la cariatide",
	"il coccodrillo",
// SCG[1/4/00]: 	"i ciclopi",
	"la sfera della morte robotizzata",
	"il pipistrello dell'Apocalisse",
	"il dragone",         // TX NOTE:must match script.
	"lo gnomo",
	"la gang di donne",
	"il traghettatore",
	"l'arciere",
	"la rana robotizzata",
	"Re Garroth",
	"il pesce d'oro",
	"il grifone",
	"l'arpia",
	"il carceriere",
	"Kage",
	"King Minos",         // TX NOTE:deleted. don't translate.
	"il primo cavaliere",
	"il secondo cavaliere",
	"la scimmia da laboratorio",
	"l'uomo del laboratorio",
	"la pistola laser robotizzata",
	"il lupo mannaro",
	"Medusa",         // TX NOTE:must match script.
	"il minotauro",
	"la guardia di Mishima",         // TX NOTE:must match script.
	"Nharre",              // TX NOTE:must match script  don't translate
	"il topo della peste",
	"il topo di fogna",
	"il sacerdote",
	"il prigioniero",
	"il proto-bozzolo robotizzato",
	"il cervello artigliato",
	"il RageMaster 5000 robotizzato",
	"il membro della gang coi razzi",
	"il poliziotto militare coi razzi",
	"il verme putrefatto",
	"il satiro",
	"il commando della Marina",         // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
// SCG[1/4/00]: 	"il sommozzatore della Marina",            // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"il capitano della Marina",         // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"la ragazza della Marina",         // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"lo squalo",
	"la zanzara assassina",
	"lo scheletro",
	"il lavoratore magro",
	"il lavoratore grasso",
	"il servo di fango robotizzato",
	"il piccolo ragno",
	"il ragno",
	"Stavros",         // TX NOTE:must match script.
	"il chirurgo",
	"il ladro",
	"la zanzara del tuono",
// SCG[1/4/00]: 	"il segugio robotizzato",
	"il membro della gang con l'Uzi",
	"il prigioniero bianco",
	"lo stregone",         // TX NOTE:must match script.
	"Wyndrax",         // TX NOTE:must match script.
	"il gabbiano",
	"il lucciola",
	"lo spiritello",
	"Mikiko"									// SCG[12/5/99]: 
};


static char *tongue_world[] =
{
	"continua",                                 // triggers to go until sequence complete TX NOTE:## +.
	"Sequenza completata",                         // trigger sequence completed TX NOTE:sentence.
	
	"Ti serve",                              // need a specific key TX NOTE:followed by the key needed.
	"Ti serve la chiave giusta\n",               // wrong key in possession TX NOTE:sentence.
	
	// ******************************************
	"Non si può aprire in questo modo\n",					       // Can't be opened this way
	// ******************************************
	
	"Il veleno sta abbandonando tuo corpo\n",    // poison expiring TX NOTE:sentence.
	"I tuoi ossigenatori sono quasi esauriti\n",           // oxylung expiring TX NOTE:sentence.
	"La tua radioprotezione si sta esaurendo\n",                // envirosuit expiring TX NOTE:sentence.
	"Il tuo megascudo si sta esaurendo\n",                 // megashield expiring TX NOTE:sentence.
	"La sfera dello spettro si sta esaurendo\n",                   // wraith orb expiring TX NOTE:sentence.
	"La potenza bonus si sta esurendo\n",				  // power boost expiring TX NOTE:sentence.
	"L'attacco bonus si sta esaurendo\n",                // attack boost expiring TX NOTE:sentence.
	"La velocità bonus si sta esaurendo\n",                 // speed boost expiring TX NOTE:sentence.
	"Il balzo bonus si sta esaurendo\n",             // jump boost expiring TX NOTE:sentence.
	"La vitalità bonus si sta esaurendo\n",              // health boost expiring TX NOTE:sentence.
	
	"si è unito alla partita",                            // player joined the game TX NOTE:NAME +.
	"è uscito dal livello",                           // player exited the level TX NOTE:NAME +.
	
	"Hiro Miyamoto",                              // Hiro Miyamoto's full name TX NOTE:same as in script.
	"Superfly Johnson",                           // Superfly's full name TX NOTE:same as in script.
	"Mikiko Ebihara",                             // Mikiko's full name TX NOTE:same as in script.
	
	"",
	"",
	"",
	"",
	"",
	
	//***********************************
	"Impossibile uscire dal livello senza\n",				// SCG[10/18/99]: no exiting level without sidekicks
	"Non puoi uscire senza\n",						// SCG[11/9/99]: No exiting without a key
	"Avvelenato!\n",								// NSS[11/29/99]: Needed a generic and yet global poisoned message

	"Hai trovato un segreto!\n",						// cek[1-3-00]
	//***********************************
	"Il teschio del mana si sta esaurendo.\n"                   // cek[2-9-00]
};

static char *tongue_deathmsg_daikatana[] = 
{
	"%s è stato fatto a fette da %s\n"
};

static char *tongue_deathmsg_self[] =
{
	" si è spaccato il cranio\n",                          // death, fell TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"è annegato\n",                             // death, drowned TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"è stato abbrustolito\n",                                // death, lava TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"è stato ricoperto di melma\n",                                 // death, slimed TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"è stato avvelenato\n",                                // death, poison TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"si è stancato di vivere\n",                             // death, killed self TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"è stato telefraggato\n",                            // death, teleport TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"è stato triturato\n",                            // death, crushed TX NOTE:character +. TX NOTE:use your own appropriate slang.
	
	"si è mangiato una cucchiaiata di C4\n"                        // death, C4 explosives TX NOTE:character +. TX NOTE:use your own appropriate slang.
};


static char *tongue_deathmsg_weapon1[] =
{
	"%s è stato umiliato da %s!",
	"%s ha spedito %s all'inferno!",
	"%s è stato ionizzato da %s!",
	"%s ha sbriciolato %s!",
	"%s ha riempito di piombo %s!",
	"%s ha cavalcato i razzi di %s!",
	"%s ha fulminato %s!"
};

static char *tongue_deathmsg_weapon2[] =
{
	"%s ha giocato a frisbee estremo con %s!",
	"%s si è abbattuto su %s!",
	"%s ha polverizzato %s!",
	"%s ha ridicolizzato %s!",
	"%s ha fatto a fette %s!",
	"%s si è inginocchiato davanti a Zeus!"
};

static char *tongue_deathmsg_weapon3[] =
{
	"%s è stato dilaniato da %s!",
	"%s è stato impalato da %s!",
	"%s ha assaggiato la furia di %s!",
	"%s è stato carbonizzato dalla meteora di %s!",
	"%s ha assaggiato l'alta tensione di %s!",
	"%s è stato massacrato dal demone di %s!"
};

static char *tongue_deathmsg_weapon4[] =
{
	"%s si è mangiato %s!",
	"%s ha devastato %s!",
	"%s è stato squarciato da %s!",
	"%s ha ridotto a brandelli %s!",
	"%s è stato congelato da %s!",
	"%s ha aperto una voragine in %s!",
	"%s è stato maserizzato da %s!"
};

static char *tongue_scoreboard[] =
{
	"Nome",
	"Uccisioni",
	"Ping",
	"Tempo",
	"Colpi",
	"Centri",
	"Morti",
	"Punti"
};


static char *tongue_difficulty[] =  // difficulty settings for 'new game' menu
{
	"Seleziona la difficoltà:",           // difficulty header TX NOTE:using karate skill levels.
	"Cintura bianca",                   // newbie
	"Cintura blu",                    // average
	"Cintura nera"                    // expert (or so they think!)
};


static char *tongue_skincolors[] =
{
	"Blu",                         // blue
	"Verde",                        // green
	"Rosso",                          // red
	"Oro"                          // gold
};


static char *tongue_statbar[] =
{
	"POTENZA",                        // skill 'power' label
	"ATTACCO",                       // skill 'attack' label
	"VELOCITÀ",												// skill 'speed' label
	"BALZO",													// skill 'acrobatic' label
	"VITALITÀ",											// skill 'vitality' label
	
	//  "CORAZZA",                        // armor label
	//  "ENERGIA",												// health label
	//  "MUNIZIONI",													// ammo label
	//  "FRAG",												// kills label
	//  "LIVELLO",  											// experience points label
	
	// not used in non-english versions
	"",																	 // armor label
	"",																	 // health label
	"",																	 // ammo label
	"",																	 // kills label
	"",																	 // experience points label
	
	
	"NUOVO LIVELLO!",                    // experience level up
	"Seleziona",               // select skill
	"Aggiungi punti",            // add point to skill
	
	//****************************************
	"CARICAMENTO..."                    // loading new level
	//****************************************
};


static char *tongue_weapon_misc[] =
{
	"I moduli C4 sono destabilizzanti!\n"    // C4 modules are de-stabilizing
};




static char *tongue_sidekick[] =    // sidekick commands
{
	"Raccogli",	                      // 'get' item command
	"Segui",                          // 'come here', 'come to me'
	"Stai qui",                       // 'stay' command, 'don't move'
	"Attacca",                        // 'attack' command
	"Non attaccare",                  // 'back off', 'move away'
	
	//******************************************
	"Potrei avere"                      //  asking to pick up an item: "Can I have the shotgun?"
	//******************************************
};


static char *tongue_ctf[] =
{
	"la Bandiera rossa",
	"la Bandiera blu",
	"Squadra sconosciuta",

	"Bandiera %s",											// when translating, treat '%s Flag' as 'red Flag' and place the %s before or after flag
														// depending on the syntax of the language.  ie if the expression should be 'Flag red' use 'Flag %s'
	"Rosso",
	"Blu",
	"Cromo",
	"Metallo",
	"Verde",
	"Arancione",
	"Viola",
	"Giallo",

	"Punteggio attuale: %s:%d, %s:%d\n",
	"Hai catturato: %s!\n",
	"%s della tua squadra ha catturato: %s!\n",
	"%s della squadra %s ha catturato la tua bandiera!\n",		// when translating, treat '%s team' as 'red team' and place the %s before or after team
															// depending on the syntax of the language.  ie if the expression should be 'team red' use 'team %s'
	"Hai afferrato: %s\n",
	"%s della tua squadra ha afferrato: %s\n",
	"%s ha rubato la tua bandiera\n",
	"%s è stata recuperata\n",

	"%d è un numero non valido. Usa 1 o 2\n",
	"Sei nella squadra %d (%s)\n",
	"Sei già nella squadra %d\n",
	"%s è entrato nella squadra %s\n",									// translate same as with the %s team above

	"La squadra %s ha vinto!\nPunteggio finale: %s:%d, %s:%d\n",		// translate same as with the %s team above
	"Pareggio!\n",
	"Tempo scaduto!\n",
	"Limite di catture raggiunto!\n"
};

static char *tongue_deathtag[] =
{
	"Bomba",														// as in backpack
	"Bomba %s",													// as in red pack
	"Il limite di punti è stato raggiunto!\n",
	"Il tuo tempo si sta esaurendo!\n",
	"Il tuo tempo è scaduto!\n",

	"%s della squadra %s ha afferrato: %s!\n",				// translate using the '%s team' guidelines above

	"La tua squadra ha conquistato un punto!\n",
	"La tua squadra ha conquistato %d punti!\n",								// you get 2 points
	"La squadra %s a conquistato un punto!\n",								// translate using the '%s team' guidelines above
	"La squadra %s ha conquistato %d punti!\n",							// The red team gets 2 points

	"Hai segnato!\n",
	"%s della tua squadra ha segnato!\n",
	"%s della squadra %s ha segnato!\n"
};
