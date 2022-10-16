// FRENCH text

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
	{"ENTREE", K_ENTER},
	{"ECHAP", K_ESCAPE},
	{"ESPACE", K_SPACE},
	{"RETOUR", K_BACKSPACE},
	{"HAUT", K_UPARROW},
	{"BAS", K_DOWNARROW},
	{"GAUCHE", K_LEFTARROW},
	{"DROITE", K_RIGHTARROW},
	
	{"ALT", K_ALT},
	{"CTRL", K_CTRL},
	{"MAJ", K_SHIFT},
	
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
	
	{"INSER", K_INS},
	{"SUPPR", K_DEL},
	{"PBAS", K_PGDN},
	{"PHAUT", K_PGUP},
	{"ORIGINE", K_HOME},
	{"FIN", K_END},
	
	{"SOURIS1", K_MOUSE1},
	{"SOURIS2", K_MOUSE2},
	{"SOURIS3", K_MOUSE3},
	
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
	
	{"PV_7",			K_KP_HOME },
	{"PV_8",		K_KP_UPARROW },
	{"PV_9",			K_KP_PGUP },
	{"PV_4",	K_KP_LEFTARROW },
	{"PV_5",			K_KP_5 },
	{"PV_6",	K_KP_RIGHTARROW },
	{"PV_1",			K_KP_END },
	{"PV_2",	K_KP_DOWNARROW },
	{"PV_3",			K_KP_PGDN },
	{"PV_ENTR",		K_KP_ENTER },
	{"PV_0",			K_KP_INS },
	{"PV_.",			K_KP_DEL },
	{"PV_/",		K_KP_SLASH },
	{"PV_-",		K_KP_MINUS },
	{"PV_+",			K_KP_PLUS },
	
	{"MOLHAUT", K_MWHEELUP },
	{"MOLBAS", K_MWHEELDOWN },
	
	{"PAUSE", K_PAUSE},
	
	{"M", ';'},	// because a raw semicolon seperates commands
	
	{NULL,0}
};

// vkey conversion table
//==========================================================================
static byte        scantokey[128] = 
                    { 
//  0           1       2       3       4       5       6       7 
//  8           9       A       B       C       D       E       F 
    0  ,	27,     '1',    '2',    '3',    '4',    '5',    '6', 
    '7',    '8',    '9',    '0',    ')',    '=',    K_BACKSPACE, 9, // 0 
    'a',    'z',    'e',    'r',    't',    'y',    'u',    'i', 
    'o',    'p',    '^',    '$',    13 ,    K_CTRL,'q',  's',      // 1 
    'd',    'f',    'g',    'h',    'j',    'k',    'l',    'm', 
    '�' ,    '`',    K_SHIFT,'\\',  'w',    'x',    'c',    'v',      // 2 
    'b',    'n',    ',',    ';',    ':',    '!',    K_SHIFT,'*', 
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
	"Vous avez ramass�",                            // weapon pickup TX NOTE:+ the items in the weapons section below.
	"Vous avez ramass�",			                // health pickup TX NOTE:+ one of the 3 health items below (health kit, Goldenorb, Wraith orb.
	"Vous avez ramass�",						    // armor good pickup TX NOTE:+ the items in the armor section below.
	"Vous avez ramass�",						    // boost pickup TX NOTE:not used - DON'T TRANSLATE.
	"Gemme de sauvegarde r�cup�r�e",            // pick up 'save gem'
	
	"FORCE",                                    // use power boost TX NOTE:must match statusbar.xls .
	"ADRESSE",                                   // use attack boost TX NOTE:must match statusbar.xls .
	"VITESSE",                                    // use speed boost TX NOTE:must match statusbar.xls .
	"AGILITE",                                     // use jump boost TX NOTE:must match statusbar.xls .
	"VITALITE",                                 // use health boost TX NOTE:must match statusbar.xls .
	
	"Va falloir se bouger !",                           // bomb explode TX NOTE:"let's get out of here!.
	"Vous avez besoin d'une bouteille.",    // need bottle to mix ingredients TX NOTE:sentence.
	"Objets n�cessaires :",                          // ingredient required TX NOTE:+ bomb item, short desc below.
	"Vous avez cr�� un explosif...",                  // bomb created TX NOTE:sentence.
	"Vous avez trouv�",                                // bomb found, TX NOTE:+ one of the ingredients or a bottle.
	"une poche de soufre",						// bomb ingredient 1
	"un morceau de charbon",                           // bomb ingredient 2
	"une fiole de salp�tre",                      // bomb ingredient 3
	"une bouteille vide",                          // bomb bottle
	
	"soufre",                                  // bomb item, short description
	"charbon",                                 // bomb item, short description
	"salp�tre",                                // bomb item, short description
	
	"Vous avez lu",                            // read book TX NOTE:+ one of two books below.
	"Vous ne pouvez pas le faire.",                // no read book TX NOTE:sentence.
	"le Livre de Sorts de Wyndrax",                    // Wyndrax spellbook
	//	"le Manuscrit Satanique",                 // Satanic book of evil TX NOTE: Removed 12/9/99
	
	"Vous avez ramass�",                            // key pickup TX NOTE:one of the 3 following items.
	"la cl� de la crypte",                            // crypt key name
	"la cl� de Wyndrax",                          // Wyndrax key name
	"une cl� de cellule",                    // Prison cell key
	
	"la carte d'acc�s rouge",						// SCG[11/15/99]: red control card name
	"la carte d'acc�s bleue",					// SCG[11/6/99]: blue control card name
	"la carte d'acc�s verte",					// SCG[11/6/99]: green control card name
	"la carte d'acc�s jaune",					// SCG[11/6/99]: yellow control card name
	"la cl� de pierre hexagonale",							// SCG[11/6/99]: hex keystone name
	"la cl� de pierre carr�e",						// SCG[11/6/99]: quad keystone name
	"la cl� de pierre triangulaire",						// SCG[11/6/99]: trigon keystone name
	"le M�gabouclier",							// SCG[11/14/99]: Megashield name
	"le Cr�ne de Mana",							// SCG[11/14/99]: 
	"l'Antidote.",						// SCG[11/14/99]: 
	"le Drachme.",								// SCG[11/14/99]: 
	"la Corne de Charon.",									// SCG[11/14/99]: 
	"une tablette avec la lettre Aplha",				// SCG[11/14/99]: A of AEGIS key name
	"une tablette avec la lettre Epsilon",				// SCG[11/14/99]: E of AEGIS key name
	"une tablette avec la lettre Gamma",				// SCG[11/14/99]: G of AEGIS key name
	"une tablette avec la lettre Iota",				// SCG[11/14/99]: I of AEGIS key name
	"une tablette avec la lettre Sigma",				// SCG[11/14/99]: S of AEGIS key name
	"un �clat du Purificateur",					// SCG[11/14/99]: 
	"le coffret noir",							// SCG[11/14/99]: 
	"l'anneau de protection contre le feu",				// SCG[11/14/99]: 
	"l'anneau de protection contre les mort-vivants",			// SCG[11/14/99]: 
	"l'anneau de protection contre la foudre",			// SCG[11/14/99]: 
	"l'envirocombinaison",							// SCG[11/14/99]: 
	
	"le kit de sant�",                           // health kit
	"l'Ame Dor�e",                           // Goldensoul
	"l'Orbe de l'ombre",							// wraith orb
	
	"une Gemme de sauvegarde",                  // 'save gem'
	"Vous n'avez pas des Gemmes des sauvegardez",		// JK [12-13-99]
	"la fiole de sant�",							   // cek[1-3-00]

	// cek[2-2-00]: altered the following ***********************************************
	"le pack de sant�",
	"le flacon de vie",
	"la flasque de sant�",
	"la caisse de sant�",
	"la bo�te de sant�",
	"Plus d'antidote.",

	// SCG[2/8/00]: messages for completed artifacts
	"Vous avez d�couvert toutes les tablettes AEGIS !",
	"Vous avez d�couvert les trois cl�s de pierre !",
	"Vous avez assembl� le Purificateur !"
};

// menu-related TX NOTE:all independent words -- no grammatical inflection.
static char *tongue_menu[] =
{
	"Avancer",                                  // key bind to Move Forward
	"Reculer",                                 // key bind to Move Backward
	"Gauche",                                // key bind to Step Left
	"Droite",                               // key bind to Step Right
	"Attaquer",                                   // key bind to Attack
	"Sauter",                                     // key bind to Jump/Up
	"Utiliser",                              // key bind to Use/Operate object
	"S'accroupir",                                   // key bind to Crouch/Down
	"Arme suivante",                              // key bind to Next Weapon
	"Arme pr�c�dente.",                              // key bind to Prev Weapon
	"Tourner gauche",                                // key bind to Turn Left
	"Tourner droite",                               // key bind to Turn Right
	"Courir",                                      // key bind to Run
	"Eviter",                                 // key bind to  Sidestep
	"Regarder haut",                                  // key bind to Look Up
	"Regarder bas",                                // key bind to Look Down
	"Centrer",                              // key bind to Center View
	"Vue souris",                                // key bind to Mouse Look
	"Vue clavier",                                  // key bind to Keyboard Look
	"Ecran plus grand",                             // key bind to increase screen size
	"Ecran plus petit",                           // key bind to decrease screen size
	"Tous les HUD",                            // key bind to Show All Huds (heads-up-displays)
	"Ouvrir Inv.",                                // key bind to Inventory Open
	"Utiliser obj.",                            // key bind to Use Item
	//  "Laisser obj.",                                // key bind to Drop Item
	"Objet suiv.",                                // key bind to Next Item
	"Objet pr�c.",                                // key bind to Prev Item
	"Permuter acolyte",                            // key bind to Swap Sidekick
	"Suivant",                             // key bind to Command Next
	"Pr�c�dent",                             // key bind to Command Prev
	"Appliquer",                            // key bind to Command Apply
	
	ARROW_LEFT_STRING  " Gauche",                 // left arrow key
	ARROW_UP_STRING    " Haut",                   // up arrow key
	ARROW_DOWN_STRING  " Bas",                 // down arrow key
	ARROW_RIGHT_STRING " Droite",                // right arrow key
	"--",                                       // no bind setting for this key
	
	"Cliquez / ENTR�E : assigner SUPPR : annuler.",     // set/delete key TX NOTE:sentence.
	"Cliquez ou ENTR�E pour s�lectionner",                 // select key TX NOTE:sentence.
	"Appuyez sur la touche d�sir�e/ECHAP pour annuler.",             // set/abort TX NOTE:sentence.
	
	"Par d�faut",                                 // default button
	
	"Aucune",                                     // no mouse bindings
	"Gauche/Droite",                          // mouse "move left and right" setting
	"Tourner Gauche/Droite",                          // mouse "turn left and right" setting
	"Regarder Haut/Bas",                             // mouse "look up and down" setting
	"Avancer/Reculer",                           // mouse "move forward and back" setting
	"Souris invers�e",                           // reverse mouse axis
	"Vue souris",                               // Mouse-Look
	"Vue glisser",                              // Mouse-Look// ?????????????
	"Axe X",                                   // x axis
	"Axe Y",                                   // y axis
	"Bouton 1",                                 // button 1 label
	"Bouton 2",                                 // button 2 label
	"Bouton 3",                                 // button 3 label
	"S�lectionnez un contr�le, ECHAP pour annuler",          // select a command, ESCape to cancel
	"Tourner G/D",                                 // turn left and right, abbreviated
	"Glisser G/D",                               // strafe left and right, abbreviated
	"Regarder H/B",                                 // look up and down, abbreviated
	"Av./Rec.",                                 // move forward and back, abbreviated
	"Sensibilit� horizontale",                            // mouse sensitivity for x-axis
	"Sensibilit� verticale",                            // mouse sensitivity for y-axis
	"Configuration souris :",                 // configure mouse options  TX NOTE:sentence.
	
	"Match � mort",                               // 'deathmatch' game
	"Jeu mortel",                                 // 'deathtag' game//???????????
	"Coop�ratif",                              // 'cooperative' game
	
	"Ronin",                                     // easy difficulty
	"Samurai",                                   // medium difficulty
	"Shogun",                                     // hard difficulty
	
	"Partie Internet",					// JK [12/9/99]
	"Partie en local",						// JK [12/9/99]
	"Lancer multijoueur",							// JK [12/9/99]
	
	"LAN",                                      // local area network game
	"Internet",                                 // internet game
	
	"Rejoindre",                                 // join server
	"H�te",                                      // host new  server
	"Ajouter " ARROW_DOWN_STRING,                   // add address
	"Actualiser",                                  // refresh network games
	"Configurer",                                    // player setup
	
	//"Handedness",                               // handedness label TX NOTE:not used. DO NOT TRANSLATE..
	//"Left",                                     // handedness, left TX NOTE:not used. DO NOT TRANSLATE..
	//"Ambi-",                                    // handedness, center TX NOTE:ambi-dextrous. TX NOTE:not used. DO NOT TRANSLATE..
	//"Right",                                    // handedness, right TX NOTE:not used. DO NOT TRANSLATE..
	
	"Connexion",                             // connection speed label
	"28.8k",                                    // connection type 1
	"55.6k",                                    // connection type 2
	"ISDN",                                 // connection type 3
	"Cable/DSL",                                // connection type 4
	"T1/LAN",                                   // connection type 5
	"Personnalis�e",                             // connection type 6 TX NOTE:user enters his own connection type.
	
	"Limte de Temps",							// JK [12/09/99]
	"Couleur Personnage",					// JK [12/09/99]
	"Lancer �pisode",						// JK [12/09/99]
	"Equipe",							// JK [12/09/99]
	"Limite de CDD",							// JK [12/09/99]
	"Couleur �quipe 1",						// JK [12/09/99]
	"Couleur �quipe 2",						// JK [12/09/99]
	"Limite de Score",							// JK [12/09/99]
	"Limite de Tu�s",                               // frag limit TX NOTE:frag = number of times you can die before re-starting the game.
	"Niveau Max",							// JK [12/09/99]
	"Lancer �pisode",						// JK [12/09/99]
	"Type de partie",                                // game mode (deathmatch,deathtag,co-op)
	"Mode de Jeu",								// JK [12/09/99]
	"Niveau de Difficult�",                              // skill level
	"Temps",                               // time limit
	"Nb/joueurs",                              // maximum players
	"Personnage",                                // character
	"Couleur de Base",                               // base color
	"Jouer !",                                   // start multiplayer game
	
	"Conserver/armes",                             // weapons remain with you into the next area of play
	"Remat�r./objets",                            // items rematerialize on the map
	"R�sur./�loign�e",                             // place user's character at the farthest point from other players
	"Quitter",                            // allow players to exit the level wihtout being killed first
	"Carte identique",                                 // use the same map as last time
	"R�sur./forc�e",                            // make the respawn command happen now.
	"D�g�ts de chute",                           // players take damage from falling down
	"Bonus instant.",                            // make instant power-ups available in the game
	"Autoriser bonus",                             // allow power-ups to be used in the game
	"Autoriser sant�",                             // allow health kits to be used in the game
	"Autoriser armure",                              // allow armor to be used in the game
	"Mun. illimit�es",                            // unlimited ammunition
	"Champ/vision",                                // limits the user's field-of-vision
	"Jeu en �quipe",                                 // team-play
	"Blesser",                           // hurt teammates allows players to cause damage to team-mates.
	"Echange rapide",                              // allows a player to change weapons faster than normal
	"Pas",                                // footsteps become audible or silent ? ? ? not sure of the translation yet
	"Grappin",                               // allow hook ?????????????
	"Montrer/douilles",                           // show shotgun shells?
	
	"Adresse IP du serveur",                        // server internet provider address
	"Aucune partie locale trouv�e",                     // no local games found TX NOTE:sentence.
	"Lancement de la carte",                             // starting map
	"Nom du serveur",                              // server name
	"Nom du joueur",                              // player name
	"Nom de l'�quipe",                                // team name
	
	"Hara-kiri ?",                        // confirm quit TX NOTE:Do not translate Harakiri--it's Jpn.
	
	"Oui",                            // 'yes' response
	"Non",                              // 'no' response
	"Sauvegarder",									//JK [12-9-99]
	
	"Charger",								//cek[12-6-99]
	"Sauvegarder",								//cek[12-6-99]
	"Effacer",							//cek[12-6-99]
	"S�lectionner fichier configuration",				//cek[12-6-99]
	"Effacer s�lection ?",						//cek[12-6-99]
	
	"Charger jeu",								//cek[12-6-99]
	"Armure ",									//cek[12-6-99]
	"Sante",									//cek[12-6-99]
	"Niveau",									//cek[12-6-99]
	"Monstres",									//cek[12-6-99]
	"Secrets",									//cek[12-6-99]
	"Temps",										//cek[12-6-99]
	"Total",									//cek[12-6-99]
	"�pisode",									//cek[12-6-99]
	
	"Son 3D",								//cek[12-6-99]
	
	"Sauvegard�",								//cek[12-7-99]
	"Charg�",									//cek[12-7-99]
	"Effac�",									//cek[12-7-99]
	
	"Souris fluide",								//cek[12-7-99]
	"Molette haut",								//cek[12-7-99]
	"Molette bas",								//cek[12-7-99]
	
	"Interdire ALT-TAB",						//cek[12-7-99]
	"Sang et entrailles",						//cek[12-7-99]
	"Armes anim�es",						//cek[12-7-99]
	"Change Armes-Auto",						//cek[12-7-99]
	
	"Lumi�re",									//cek[12-7-99]
	"Reflets armes",							//cek[12-7-99]
	"Impacts",								// SCG[1/4/00]: JK [2/8/00]
	
	"Avancer",									//cek[12-9-99]
	"Lat�ral",										//cek[12-9-99]
	"Haut",										//cek[12-9-99]
	"Vers le Haut",									//cek[12-9-99]
	"D�vier",										//cek[12-9-99]
	"Utiliser joystick",								//cek[12-9-99]
	"Configurer l'axe",							//cek[12-9-99]
	"Configurer joystick",						//cek[12-9-99]
	"Choisissez un action pour cette bouton",			//cek[12-9-99]
	"Appuyez sur le bouton du joystick d�sir�",			//cek[12-9-99]
	"Seuil",								//cek[12-9-99]
	"Sensibilit�",								//cek[12-9-99]
	
	"Capture du Drapeau",							//cek[12-10-99]
	"Returne",										//cek[12-10-99]
	"56.6k",									//cek[12-10-99]
	"Auto",										//cek[12-10-99]
	"Equipe 1",									//cek[12-10-99]
	"Equipe 2",									//cek[12-10-99]
	
	"Jouer film",								// JK [12-13-99]
	"Utilisez/Abilit�",							// JK [12-13-99]
	"Vitesse de souris",							// JK [12-13-99]
	"RAMASSE",								// cek[1-3-00]
	"AMENE-TOI",							// cek[1-3-00]
	"ATTAQUE",							// cek[1-3-00]
	"ATTAQUE PAS",							// cek[1-3-00]
	"REST'LA",							// cek[1-3-00]

	"S�lection arme %d",							// cek[1-4-00]  as in "Select Weapon 1" (leave the %d)

	"DEFIL. HUD",								// cek[1-5-00]  -- for these, translate HUD to something short and reasonably
	"HUD SUIV.",									// cek[1-5-00]  -- similar in meaning.
	"HUD PREC.",									// cek[1-5-00]
	"UTILISER HUD",									// cek[1-5-00]

	"Discuter",										// cek[1-11-00]
	"Scores",								// SCG[1/13/00]: 
	"Message �quipe",								// cek[1-22-00]
	"%s sert d�j� pour %s",				// cek[1-22-00]  as in "F is already assigned to attack" (leave in %s)
	"Remplacer ?",									// cek[1-22-00]

	"Dir. Lobby",								// cek[2-1-00]
	"Infos DK",								// cek[2-1-00]
	"Continuer ?",								// cek[2-1-00]

	// for the next two, all \n in the strings denotes a string break.  These must remain in place and the translated
	// string should have them in roughly the same place.
	"Quitter Da�katana\net lancer le serveur Mplayer ?",	// cek[2-1-00]
	"Infos, Forums et Classements\nsur www.daikatananews.com.\nLancer le navigateur Internet ?",			// cek[2-1-00]

	"Arme %d",									// cek[3-6-00]: treat as "weapon 1"
	"Annulent",									// cek[3-9-00]
	"www.planetdaikatana.com",													// cek[3-9-00]
	"Aller au www.planetdaikatana.com.\nLancer le navigateur Internet ?",			// cek[3-9-00]
};


static char *tongue_menu_options[] =
{
	"Toujours courir",                               // makes user's characters run everywhere
	"Vue en saut",                               // look + jump ????????????
	"Vue rafale",                               // look + strafe (shoot wildly) ? ? ? ? ? ? ?
	"Vis�e automatique",                           // computer aims at nearest target for the user
	"Mire",                                // a crosshair is displayed
	"Arme visible",                           // weapon is shown
	"Sous-titres",                                // subtitles
	
	"Volume des effets sonores",                      // volume for sound fx
	"Volume de la musique",                             // volume for music
	"Volume des films",                         // volume for cinematics
	
	"Logiciel",                                 // video mode, software
	"OpenGL",                                   // video mode, opengl
	"Appliquer modifications",                            // apply video mode changes TX NOTE:sentence.
	
	"Plein �cran",                               // make view use the full screen
	//	"Stipple Alpha",                            // stipple alpha TX NOTE:this is a type of speed optimization technique. Don't translate.
	"Ombres",                                  // allow shadows
	"Pilote",                                   // video driver
	"R�solution",                               // choose/set screen display resolution
	"Taille de l'�cran",                              // choose/set screen size
	"Luminosit�",                               // choose/set brightness
	"Qualit� textures",                          // choose/set quality of 3D image textures
	"Afficher barre d'�tat",                    // choose/set whether the status bar is visible.
	"Effets Brouillard / eau",								// tessellation
	"Ronin",
	"Samurai",
	"Shogun",

	"Neige / Pluie",							// cek[1-11-00]
	"Brouillard"										// cek[1-11-00]		
};



//descriptions for the texture quality settings.
static char *texture_quality_desc[] =
{
	"Totale",
	"Moiti�",
	"Quart",
	"Huiti�me"
};


// weapon names TX NOTE:"you picked up" +.TX NOTE:translate what you can. "localize" spellings of anything else.
static char *tongue_weapons[] =
{
	"plus de munitions",		//only shows up if you have no ammo for the weapon you're trying to use. TX NOTE:treat like sentence "you have "no ammo".
	"la Baliste",
	"des Carreaux de Baliste",
	"l'Arbal�te",
	"les Carreaux",
	"le C4 Vizatergo",
	"les charges de C4",
	//	"le Celestril",
	//	"le Bouclier Celestril",
	"le Da�katana",
	//	"le Da�katana Bannisseur",
	//	"le Da�katana Bouclier",
	//	"le Da�katana Dansant",
	//	"le Da�katana",
	//	"le Da�katana Poss�d�",
	//	"le Da�katana Prot�e",
	"le Disque de D�dale",
	"le Disque",
	"le Gant de Perturbation",
	//	"la Lampe de Poche",
	"l'Automark 2020", //
	"les Balles",
	"le Marteau",
	"le Blaster � ions",
	"des balles ionisantes",
	"le Cin�ticore",
	"la Boule clout�e",//
	"le M�tamaseur",
	"le M�tamaseur",
	//	"la Main Dor�e de Midas",
	//	"la Pyrite",
	"le Cauchemar de Nharre",
	"les Entrailles", 
	"le Rayon Nova",
	"la charge pour Nova",
	"le M�gashotgun",
	"les Grenades � cordite",
	"l'Onde de Choc",
	"la Sph�re de choc",
	"l'Autoshotgun",
	"les cartouches",
	"le Sidewinder",
	"les Fus�e de Sidewinder",
	"les Griffes Argent�es",
	"la D�vastatrice",
	"les Bastos",
	"le B�ton de Stavros",
	"les Rochers de Lave",
	"la Br�lure Solaire",
	"la Br�lure Solaire",
	// "le Crochet de Tazer",
	"le Trident de Pos�idon",
	"les pointes du Trident.",
	"le B�ton serpent.",
	"le venin de cobra.",
	"le Globe de Wyndrax",
	"les D�charges",//
	"l'oeil de Zeus.",
	"l'Eclair.",
	"les Mains de l'Apocalypse"//
};

// armor TX NOTE:"you picked up" +.TX NOTE:translate what you can. "localize" spellings of anything else.
static char *tongue_armors[] =
{
	"l'Armure de plastiacier.",
	"l'Armure chromatique.",
	"l'Armure d'argent.",
	"l'Armure d'or.",
	"la Cotte de mailles.",
	"l'Armure noire.",
	"l'Armure de kevlar.",
	"l'Armure d'�bonite."
};


// monster names TX NOTE:NOMINITIVE CASE. translate what you can. "localize" spellings of anything else.
static char *tongue_monsters[] =
{
	"Le Sanglier de Combat RoboCo",
	"Le Prisonnier Noir",
	"Le Buboid",         
	"Le Robocame RoboCo",
	"Le Centurion",
	"Cerberus",          // TX NOTE:deleted. don't translate.
	"L'Artilleur Volant",
	"La Colonne Caratyde",
	"Le Crox",
	//	"Les Cyclopes",
	"La Sph�re Mortelle RoboCo",
	"La Chauve-Souris de l'Apocalypse",
	"Le Dragon",         
	"Le Nain",
	"Le Gang des Femmes",
	"Le Passeur",
	"Le Fletcher",
	"Le Crapauteur RoboCo",
	"Le Roi Garroth",
	"Le Poisson Rouge",
	"Le Griffon",
	"La Harpie",
	"L'Intern�",
	"Kage",
	"Le Roi Minos",         // TX NOTE:deleted. don't translate.
	"Le Premier Chevalier",
	"Le Second Chevalier",
	"Le Singe de Laboratoire",
	"Le Laborantin",
	"Le Laser RoboCo",
	"Lycanthir",//
	"M�duse",         
	"Le Minotaure",
	"Le Garde Mishima",         
	"Nharre",                   
	"Le Rat de la Peste",
	"Le Rat d'�gout"
	"Le Pr�tre",
	"Le Prisonnier",
	"Le Protopode RoboCo",
	"La Pince Psychique",
	"Le RageMaster 5000 Roboco",
	"Le Membre du Gang des Roquettes",
	"Le PM Roquettes",
	"Le Ver Putr�fi�",
	"Le Satyre",
	"Le Commando SEAL",         
	//	"Le Plongeur SEAL",            
	"Le Capitaine SEAL",         
	"La SEAL",         
	"Le Requin",
	"Le Boucher",
	"Le Squelette",
	"L'Ouvrier Emaci�",
	"L'Ouvrier Ob�se",
	"La Cr�ature des Egouts Roboco",
	"La Petite Araign�e",
	"L'Araign�e",
	"Stavros",        
	"Le Chirurgien",
	"Le Voleur",
	"Le Tonnerre",
	//	"L'Agresseur Roboco",
	"Le Membre du Gang Uzi",
	"Le Prisonnier Blanc",
	"Le Sorcier",         // TX NOTE:must match script.
	"Wyndrax",         // TX NOTE:must match script.
	"Mouette",
	"Papillon",
	"Eph�m�re",
	"Mikiko"
};

// monster names ACCUSATIVE CASE. NOT USED IN ENGLISH. translate what you can. "localize" spellings of anything else.
static char *tongue_monsters_acc[] =
{
	"le Sanglier de Combat RoboCo",
	"le Prisonnier Noir",
	"le Buboid",         // TX NOTE:must match script.
	"le Robocame RoboCo",
	"le Centurion",
	"the Cerberus",         // TX NOTE:deleted. don't translate.
	"l'Artilleur Volant",
	"la Colonne Caratyde",
	"le Crox",
	//	"les Cyclopes",
	"la Sph�re Mortelle RoboCo",
	"la Chauve-Souris de l'Apocalypse",
	"le Dragon",         // TX NOTE:must match script.
	"le Nain",
	"le Gang des Femmes",
	"le Passeur",
	"le Fletcher",//????????
	"le Crapauteur RoboCo",
	"le Roi Garroth",
	"le Poisson Rouge",
	"le Griffon",
	"la Harpie",
	"l'Intern�",
	"Kage",
	"le Roi Minos",         // TX NOTE:deleted. don't translate.
	"le Premier Chevalier",
	"le Second Chevalier",
	"le Singe de Laboratoire",
	"le Laborantin",
	"le Laser RoboCo",
	"le Lycanthir",
	"M�duse",         // TX NOTE:must match script.
	"le Minotaure",
	"le Garde Mishima",         // TX NOTE:must match script.
	"Nharre",              // TX NOTE:must match script  don't translate
	"le Rat de la Peste",
	"le Rat d'�gout"
	"le Pr�tre",
	"le Prisonnier",
	"le Protopode RoboCo",
	"la Pince Psychique",
	"le RageMaster 5000 RoboCo",
	"le Membre du Gang des Roquettes",
	"le PM Roquettes",
	"le Ver Putr�fi�",
	"le Satyre",
	"le Commando SEAL",         // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	//	"le Plongeur SEAL",            // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"le Capitaine SEAL",         // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"la SEAL",         // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"le Requin",
	"le Boucher",//?????
	"le Squelette",
	"l'Ouvrier Emaci�",
	"l'Ouvrier Ob�se",
	"la Cr�ature des Egouts Roboco",
	"la Petite Araign�e",
	"l'Araign�e",
	"Stavros",         // TX NOTE:must match script.
	"le Chirurgien",
	"le Voleur",
	"le Tonnerre",//????????
	//	"l'Agresseur Roboco",//??????
	"le Membre du Gang Uzi",
	"le Prisonnier Blanc",
	"le Sorcier",         // TX NOTE:must match script.
	"Wyndrax",         // TX NOTE:must match script.
	"Mouette",
	"Papillon",
	"Eph�m�re",
	"Mikiko"
};


static char *tongue_world[] =
{
	"encore",                                 // triggers to go until sequence complete TX NOTE:## +.
	"S�quence termin�e ",                         // trigger sequence completed TX NOTE:sentence.
	
	"Vous devez avoir",                              // need a specific key TX NOTE:followed by the key needed.
	"Vous avez besoin de la cl� correspondante.\n",               // wrong key in possession TX NOTE:sentence.
	"Ouverture impossible de cette fa�on.\n",					       // Can't be opened this way

	"Vous sentez que le poison se dissipe.\n",    // poison expiring TX NOTE:sentence.
	"Vos oxypoumons sont pratiquement hors d'�tat.\n",           // oxylung expiring TX NOTE:sentence.
	"Votre envirocombinaison est pratiquement hors d'�tat.\n",                // envirosuit expiring TX NOTE:sentence.
	"Votre M�gabouclier est pratiquement hors d'�tat.\n",                 // megashield expiring TX NOTE:sentence.
	"L'Orbe de l'ombre bl�mit.\n",                   // wraith orb expiring TX NOTE:sentence.
	"Le Booster de Force s'�puise.\n",				  // power boost expiring TX NOTE:sentence.
	"Le Booster d'Adresse s'�puise.\n",                // attack boost expiring TX NOTE:sentence.
	"Le Booster de Vitesse s'�puise.\n",                 // speed boost expiring TX NOTE:sentence.
	"Le Booster d'Agilit� s'�puise.\n",             // jump boost expiring TX NOTE:sentence.
	"Le Booster de Vitalit� s'�puise.\n",              // health boost expiring TX NOTE:sentence.
	
	"vient d'entrer la partie.",                            // player joined the game TX NOTE:NAME +.
	"a quitt� le niveau",                           // player exited the level TX NOTE:NAME +.
	
	"Hiro Miyamoto",                              // Hiro Miyamoto's full name TX NOTE:same as in script.
	"Superfly Johnson",                           // Superfly's full name TX NOTE:same as in script.
	"Mikiko Ebihara",                             // Mikiko's full name TX NOTE:same as in script.
	
	"",
	"",
	"",
	"",
	"",
	
	"Impossible de quitter ce niveau sans\n",			// SCG[10/18/99]: no exiting level without sidekicks 
	"Impossible de sortir d'ici sans\n",						// SCG[11/9/99]: No exiting without a key
	"Empoisonn� !\n",

	"Vous avez d�couvert un passage secret !\n",						// cek[1-3-00]
	"Le Cr�ne de Mana s'�puise.\n"                   // cek[2-9-00]
};

static char *tongue_deathmsg_daikatana[] = 
{
	"%s a �t� d�coup� en rondelles par %s\n"
};

static char *tongue_deathmsg_self[] =
{
	"s'est fracass� le cr�ne.\n",                          // death, fell TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"s'est noy�(e).\n",                             // death, drowned TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"est carbonis�(e).\n",                                // death, lava TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"est tout(e) gluant(e).\n",                                 // death, slimed TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"s'est fait pourrir.\n",                                // death, poison TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"votre vie s'en est all�e...\n",                             // death, killed self
	"a eu un p�pin de t�l�porteur.\n",                            // death, teleport TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"s'est fait �craser.\n",                            // death, crushed TX NOTE:character +. TX NOTE:use your own appropriate slang.
	
	"dig�re mal le C4.\n"                        // death, C4 explosives TX NOTE:character +. TX NOTE:use your own appropriate slang.
};


static char *tongue_deathmsg_weapon1[] =
{
	"%s s'est fait humilier par %s !",
	"%s a salement mat� %s !",
	"%s a �t� ionis� par %s !",
	"%s a transform� %s en tas de boue !",
	"%s a pulv�ris� %s !",
	"%s a go�t� aux roquettes de %s !",
	"%s a latt� %s !"
};

static char *tongue_deathmsg_weapon2[] =
{
	"%s a jou� au Frisbee infernal avec %s !",
	"%s est tomb� sur %s !",
	"%s a transform� %s en miasmes !",
	"%s s'est expliqu� avec %s !",
	"%s a d�compos� %s !",
	"%s s'est agenouill� devant Zeus !"
};

static char *tongue_deathmsg_weapon3[] =
{
	"%s s'est fait d�couper comme un jambon par %s !",
	"%s s'est f�ch� avec %s !",
	"%s a go�t� � la col�re de %s !",
	"%s a �t� la victime de l'apocalypse selon %s !",
	"%s s'est fait branch� par %s !",
	"%s aurait besoin d'un exorciste, merci %s !"
};

static char *tongue_deathmsg_weapon4[] =
{
	"%s a eu une discussion d'hommes avec %s !",
	"%s a mis %s sur orbite !",
	"%s s'est fait �ventr� par %s !",
	"%s a �charp� %s !",
	"%s s'est fait geler par %s !",
	"%s a fait de jolis trous dans %s !",
	"%s a trouv� son ma�tre, %s !"
};

static char *tongue_scoreboard[] =
{
	"Nom",
	"Tu�s",
	"Ping",
	"Temps",
	"Tirs",
	"Touch�s",
	"Mort",
	"Score"
};


static char *tongue_difficulty[] =  // difficulty settings for 'new game' menu
{
	"Niveau de difficult� :",           // difficulty header TX NOTE:using karate skill levels.
	"Ceinture blanche",                   // newbie
	"Ceinture bleue",                    // average
	"Ceinture noire"                    // expert (or so they think!)
};


static char *tongue_skincolors[] =
{
	"Bleu",                         // blue
	"Vert",                        // green
	"Rouge",                          // red
	"Dor�"                          // gold
};


static char *tongue_statbar[] =
{
	"FORCE",                        // skill 'power' label
	"ADRESSE",                       // skill 'attack' label
	"VITESSE",												// skill 'speed' label
	"AGILITE",													// skill 'acrobatic' label
	"VITALITE",											// skill 'vitality' label
	
	//	"ARMURE",                        // armor label
	//  "SANTE",												// health label
	//  "MUNITIONS",													// ammo label
	//  "TU�S",												// kills label
	//  "NIVEAU",  											// experience points label
	
	// not used in non-english versions
	"",																	 // armor label
	"",																	 // health label
	"",																	 // ammo label
	"",																	 // kills label
	"",																	 // experience points label
	
	"NIVEAU +1!",                    // experience level up
	"S�lection",               // select skill
	"Ajouter point",            // add point to skill
	
	"CHARGEMENT..."                   // loading new level
};


static char *tongue_weapon_misc[] =
{
	"Les charges de C4 sont instables !\n"    // C4 modules are de-stabilizing
};


static char *tongue_sidekick[] =    // sidekick commands
{
	"Ramasse",		                        // 'get' item command
	"Amene-toi",                           // 'come here', 'come to me'
	"Rest'la",                           // 'stay' command, 'don't move'
	"Attaque",                         // 'attack' command
	"Attaque-pas",                        // 'back off', 'move away'
	
	"Je peux avoir"                         // requesting to pick up an item
};

static char *tongue_ctf[] =
{
	"Drapeau rouge",
	"Drapeau bleu",
	"Inconnu",

	"%s Flag",											// when translating, treat '%s Flag' as 'red Flag' and place the %s before or after flag
														// depending on the syntax of the language.  ie if the expression should be 'Flag red' use 'Flag %s'
	"Rouge",
	"Bleu",
	"Chrome",
	"M�tal",
	"Vert",
	"Orange",
	"Violet",
	"Jaune",

	"Score: %s:%d, %s:%d\n",
	"Vous avez captur� le %s!\n",
	"Votre co�quipier %s a captur� le %s!\n",
	"%s de l'�quipe %s a captur� votre drapeau!\n",		// when translating, treat '%s team' as 'red team' and place the %s before or after team
															// depending on the syntax of the language.  ie if the expression should be 'team red' use 'team %s'
	"Vous avez ramass� le %s\n",
	"Votre co�quipier %s a ramass� le %s\n",
	"%s a vol� votre drapeau\n",
	"Le %s a �t� r�cup�r�\n",

	"%d n'est pas un chiffre accept�. Utilisez 1 ou 2\n",
	"Vous faites partie de l'�quipe %d (%s)\n",
	"Vous �tes d�j� dans l'�quipe %d\n",
	"%s a rejoint l'�quipe %s\n",									// translate same as with the %s team above

	"Victoire de l'�quipe %s!\nScore final: %s :%d, %s :%d\n",		// translate same as with the %s team above
	"C'est un match nul!\n",
	"Temps �coul�!\n",
	"Limite de capture atteinte!\n"
};

static char *tongue_deathtag[] =
{
	"Bombe",														// as in backpack
	"Bombe %s",													// as in red pack
	"Limite de score atteinte!\n",
	"Le temps s'�gr�ne!\n",
	"C'est fini!\n",

	"%s de l'�quipe %s a ramass� %s!\n",				// translate using the '%s team' guidelines above

	"Un point pour votre �quipe!\n",
	"%d points pour votre �quipe!\n",								// you get 2 points
	"Un point pour l'�quipe %s!\n",								// translate using the '%s team' guidelines above
	"%d points pour l'�quipe %s!\n",							// The red team gets 2 points

	"Un point pour vous!\n",
	"Un point pour %s!\n",
	"Un point pour %s de l'�quipe %s!\n"
};





