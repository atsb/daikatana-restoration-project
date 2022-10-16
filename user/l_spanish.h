// SPANISH text

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
	{"INTRO", K_ENTER},
	{"ESC", K_ESCAPE},
	{"ESPACIO", K_SPACE},
	{"RETROCESO", K_BACKSPACE},
	{"ARRIBA", K_UPARROW},
	{"ABAJO", K_DOWNARROW},
	{"IZQUIERDA", K_LEFTARROW},
	{"DERECHA", K_RIGHTARROW},
	
	{"ALT", K_ALT},
	{"CONTROL", K_CTRL},
	{"MAYÚS", K_SHIFT},
	
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
	
	{"INSERT", K_INS},
	{"SUPR", K_DEL},
	{"AVPÁG", K_PGDN},
	{"REPÁG", K_PGUP},
	{"INICIO", K_HOME},
	{"FIN", K_END},
	
	{"RATÓN1", K_MOUSE1},
	{"RATÓN2", K_MOUSE2},
	{"RATÓN3", K_MOUSE3},
	
	{"JOYSTICK1", K_JOY1},
	{"JOYSTICK2", K_JOY2},
	{"JOYSTICK3", K_JOY3},
	{"JOYSTICK4", K_JOY4},
	
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
	
	{"TEC_INICIO",			K_KP_HOME },
	{"TEC_ARRIBA",		K_KP_UPARROW },
	{"TEC_REPÁG",			K_KP_PGUP },
	{"TEC_IZQUIERDA",	K_KP_LEFTARROW },
	{"TEC_5",			K_KP_5 },
	{"TEC_DERECHA",	K_KP_RIGHTARROW },
	{"TEC_FIN",			K_KP_END },
	{"TEC_ABAJO",	K_KP_DOWNARROW },
	{"TEC_AVPÁG",			K_KP_PGDN },
	{"TEC_INTRO",		K_KP_ENTER },
	{"TEC_INSERT",			K_KP_INS },
	{"TEC_SUPR",			K_KP_DEL },
	{"TEC_BARRA",		K_KP_SLASH },
	{"TEC_MENOS",		K_KP_MINUS },
	{"TEC_MÁS",			K_KP_PLUS },
	
	{"SUBIR", K_MWHEELUP },
	{"BAJAR", K_MWHEELDOWN },
	
	{"PAUSA", K_PAUSE},
	
	{"PUNTO Y COMA", ';'},	// because a raw semicolon seperates commands
	
	{NULL,0}
};

// vkey conversion table
//==========================================================================
static byte        scantokey[128] = 
                    { 
//  0           1       2       3       4       5       6       7 
//  8           9       A       B       C       D       E       F 
    0  ,	27,     '1',    '2',    '3',    '4',    '5',    '6', 
    '7',    '8',    '9',    '0',    '\'',    '¡',    K_BACKSPACE, 9, // 0 
    'q',    'w',    'e',    'r',    't',    'y',    'u',    'i', 
    'o',    'p',    '`',    '+',    13 ,    K_CTRL,'a',  's',      // 1 
    'd',    'f',    'g',    'h',    'j',    'k',    'l',    'ñ', 
    '´' ,    '`',    K_SHIFT,'ç',  'z',    'x',    'c',    'v',      // 2 
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
	"Has recogido",                            // weapon pickup TX NOTE:+ the items in the weapons section below.
	"Has recogido",			                // health pickup TX NOTE:+ one of the 3 health items below (health kit, Goldenorb, Wraith orb.
	"Has recogido",						    // armor good pickup TX NOTE:+ the items in the armor section below.
	"Has recogido",						    // boost pickup TX NOTE:not used - DON'T TRANSLATE.
		
	// **************************************************************
	"¡Obtenida la gema para guardar la partida",             // pick up 'save gem'
	// **************************************************************
		
		
	"POTENCIA",                                    // use power boost TX NOTE:must match statusbar.xls .
	"ATAQUE",                                   // use attack boost TX NOTE:must match statusbar.xls .
	"VELOCIDAD",                                    // use speed boost TX NOTE:must match statusbar.xls .
	"ACROBACIA",                                     // use jump boost TX NOTE:must match statusbar.xls .
	"VITALIDAD",                                 // use health boost TX NOTE:must match statusbar.xls .
		
	"¡Vámonos volando!",                           // bomb explode TX NOTE:"let's get out of here!.
	"Necesitas una botella para mezclar eso.",    // need bottle to mix ingredients TX NOTE:sentence.
	"Objetos necesarios: ",                          // ingredient required TX NOTE:+ bomb item, short desc below.
	"Has fabricado un explosivo...",                  // bomb created TX NOTE:sentence.
	"Has encontrado",                                // bomb found, TX NOTE:+ one of the ingredients or a bottle.
	"una bolsita de azufre",						// bomb ingredient 1
	"un trozo de carbón",                           // bomb ingredient 2
	"un frasco de nitrato de sodio",                      // bomb ingredient 3
	"una botella vacía",                          // bomb bottle
		
	"azufre",                                  // bomb item, short description
	"carbón",                                 // bomb item, short description
	"nitrato de sodio",                                // bomb item, short description
		
	"Lees ",                            // read book TX NOTE:+ one of two books below.
	"Esto no se puede usar aquí.",                // no read book TX NOTE:sentence.
	"el libro de hechizos de Wyndrax",                    // Wyndrax spellbook
//		"el libro satánico del Mal",                 // Satanic book of evil
		
	"Has recogido",                            // key pickup TX NOTE:one of the 3 following items.
	"la llave de la cripta",                            // crypt key name
	"la llave de Wyndrax",                          // Wyndrax key name
	"una tarjeta de acceso a una celda",                    // Prison cell key
		
	"la tarjeta de control roja",						// SCG[11/15/99]: red control card name
	"la tarjeta de control azul",					// SCG[11/6/99]: blue control card name
	"la tarjeta de control verde",					// SCG[11/6/99]: green control card name
	"la tarjeta de control amarilla",					// SCG[11/6/99]: yellow control card name
	"la clave hexagonal",							// SCG[11/6/99]: hex keystone name
	"la clave cuadrangular",						// SCG[11/6/99]: quad keystone name
	"la clave triangular",						// SCG[11/6/99]: trigon keystone name
	"el megaescudo",							// SCG[11/14/99]: Megashield name
	"la calavera mágica",							// SCG[11/14/99]: 
	"el antídoto del veneno",						// SCG[11/14/99]: 
	"el Dracma",								// SCG[11/14/99]: 
	"el cuerno de Caronte",						// SCG[11/14/99]: 
	"la letra A de la clave AEGIS",				// SCG[11/14/99]: A of AEGIS key name
	"la letra E de la clave AEGIS",				// SCG[11/14/99]: E of AEGIS key name
	"la letra G de la clave AEGIS",				// SCG[11/14/99]: G of AEGIS key name
	"la letra I de la clave AEGIS",				// SCG[11/14/99]: I of AEGIS key name
	"la letra S de la clave AEGIS",				// SCG[11/14/99]: S of AEGIS key name
	"un casco purificador",					// SCG[11/14/99]: 
	"el arca negra",							// SCG[11/14/99]: 
	"el anillo contra el fuego",				// SCG[11/14/99]: 
	"el anillo de la inmortalidad",			// SCG[11/14/99]: 
	"el anillo contra el relámpago",			// SCG[11/14/99]: 
	"el entorno-traje",							// SCG[11/14/99]: 
	
	"el botiquín",                           // health kit
	"el alma dorada",                           // Goldensoul
	"la esfera espectral",							// wraith orb
		
	// *******************************************************
	"una piedra preciosa de salvación",								// T_SAVEGEM
	"No tienes piedra preciosa de salvación",					//cek[12-13-99]
	"el frasco de vitalidad",							// cek[1-3-00]
	// *******************************************************

	// cek[2-2-00]: altered the following ***********************************************
	"el paquete de vitalidad",
	"el vaso de la vida",
	"la botella de vitalidad",
	"el cajón de vitalidad",
	"la caja de vitalidad",
	"Se ha terminado el antídoto.",

	"¡You found all the AEGIS runes!",
	"¡You found all three keystones!",
	"You have assembled the Purifier"
};



// menu-related TX NOTE:all independent words -- no grammatical inflection.
static char *tongue_menu[] =
{
	"Adelante",                                  // key bind to Move Forward
	"Atrás",                                 // key bind to Move Backward 
	"Paso/izquierda",                                // key bind to Step Left
	"Paso/derecha",                               // key bind to Step Right
	"Atacar",                                   // key bind to Attack
	"Saltar",                                     // key bind to Jump/Up
	"Usar/manipular",                              // key bind to Use/Operate object
	"Agacharse",                                   // key bind to Crouch/Down
	"Arma siguiente",                              // key bind to Next Weapon
	"Arma anterior",                              // key bind to Prev Weapon
	"Girar/izquierda",                                // key bind to Turn Left
	"Girar/derecha",                               // key bind to Turn Right
	"Correr",                                      // key bind to Run
	"Paso lateral",                                 // key bind to  Sidestep
	"Mirar arriba",                                  // key bind to Look Up
	"Mirar abajo",                                // key bind to Look Down
	"Centrar vista",                              // key bind to Center View
	"Mirar con ratón",                                // key bind to Mouse Look
	"Mirar con teclas",                                  // key bind to Keyboard Look
	// ********************************************* 
	"Aumentar vista",                             // key bind to Screen size up
	"Disminuir vista",                           // key bind to Screen size down
	"Mostrar HUD",                            // key bind to Show All Huds
	// *********************************************
	"Abrir inventario",                                // key bind to Inventory Open
	"Usar objeto",                            // key bind to Use Item
	//  "Soltar objeto",                                // key bind to Drop Item
	"Objeto siguiente",                                // key bind to Next Item
	"Objeto anterior",                                // key bind to Prev Item
	"Cambiar compañero",                            // key bind to Swap Sidekick
	"Siguiente comando",                             // key bind to Command Next
	"Comando anterior",                             // key bind to Command Prev
	"Aplicar comando",                            // key bind to Command Apply
		
	ARROW_LEFT_STRING  " Izquierda",                 // left arrow key
	ARROW_UP_STRING    " Arriba",                   // up arrow key
	ARROW_DOWN_STRING  " Abajo",                 // down arrow key
	ARROW_RIGHT_STRING " Derecha",                // right arrow key
	"--",                                       // no bind setting for this key
		
	"INTRO para asignar, SUPR para borrar",     // set/delete key TX NOTE:sentence.
	"INTRO para seleccionar",                 // select key TX NOTE:sentence.
	"asigna tecla/ESC para cancelar",             // set/abort TX NOTE:sentence.
		
	"Por defecto",                                 // default button
		
	"Ninguna",                                     // no mouse bindings
	"Moverse a la izquierda/derecha",                          // mouse "move left and right" setting
	"Girar a la izquierda/derecha",                          // mouse "turn left and right" setting
	"Mirar arriba/abajo",                             // mouse "look up and down" setting
	"Moverse adelante/atrás",                           // mouse "move forward and back" setting
	"Invertir eje Y",                           // reverse mouse axis
	"Mirar con ratón",                               // Mouse-Look
	"Mirar con desplazamiento",                              // Mouse-Look
	"Eje X",                                   // x axis
	"Eje Y",                                   // y axis
	"Botón 1",                                 // button 1 label
	"Botón 2",                                 // button 2 label
	"Botón 3",                                 // button 3 label
	"Selecciona un comando/ESC para cancelar",          // select a command, ESCape to cancel
	"Girar I/D",                                 // turn left and right, abbreviated
	"Despl. lat. I/D",                               // strafe left and right, abbreviated
	"Mirar arr./abajo",                                 // look up and down, abbreviated
	"Mov. adel./atrás",                                 // move forward and back, abbreviated
	"Sensibilidad eje X",                            // mouse sensitivity for x-axis
	"Sensibilidad eje Y",                            // mouse sensitivity for y-axis 
	"Configurar opciones del ratón:",                 // configure mouse options  TX NOTE:sentence.
	
	"Combate a muerte",                               // 'deathmatch' game
	"Contacto mortal",                                 // 'deathtag' game
	"Cooperativo",                              // 'cooperative' game
		
	"Fácil",                                     // easy difficulty
	"Media",                                   // medium difficulty
	"Difícil",                                     // hard difficulty
		
	// ******************************************
	"Buscar juego en la red",					// SCG[12/1/99]: 
	"Buscar juego en LAN",						// SCG[12/1/99]: 
	"Partida multijugador",						// SCG[12/1/99]: 
	"LAN",                                      // local area network game
	"Internet",                                 // internet game
	// ******************************************
		
		
	"Unirse",                                     // join server
		
	// ******************************************
	"Host",                                      // host new server
	// ******************************************
		
		
	"Añadir " ARROW_DOWN_STRING,                   // add address
	"Actualizar",                                  // refresh network games
	"Configurar",                                    // player setup 
		
	//"Handedness",                               // handedness label TX NOTE:not used. DO NOT TRANSLATE..
	//"Left",                                     // handedness, left TX NOTE:not used. DO NOT TRANSLATE..
	//"Ambi-",                                    // handedness, center TX NOTE:ambi-dextrous. TX NOTE:not used. DO NOT TRANSLATE..
	//"Right",                                    // handedness, right TX NOTE:not used. DO NOT TRANSLATE..
		
	"Velocidad de conexión",                             // connection speed label
	"28.800",                                    // connection type 1
	"56.600",                                    // connection type 2
	"RDSI",										  // connection type 3
	"Cable/DSL",                                // connection type 4
	"T1/Red local",                                   // connection type 5
	"Definida por el usuario",                             // connection type 6 TX NOTE:user enters his own connection type.
		
	"Límite de tiempo",								// SCG[12/2/99]: 
	"Color del personaje",							// SCG[12/2/99]: 
	"Episodio de comienzo",							// SCG[12/2/99]: 
	"Elegir equipo",								// SCG[12/1/99]: 
	"Límite CLB",								// SCG[12/1/99]: 
	"Color equipo 1",								// SCG[12/1/99]: 
	"Color equipo 2",								// SCG[12/1/99]: 
	"Puntuación límite",								// SCG[12/1/99]: 
	"Muertes", 								// SCG[12/1/99]: 
	"Límite de nivel",								// level limit
	"Iniciar episodio",							// SCG[12/2/99]: 
	"Mapa de juego",									// SCG[12/1/99]: 
	"Modo de juego",                                // game mode (deathmatch,deathtag,co-op)
	"Nivel de destreza",                              // skill level
	"Tiempo",                               // time limit
	"Nº jugadores",                              // maximum players
	"Personaje",                                // character
	"Color base",                               // base color
	"¡Empezar!",                                   // start multiplayer game
		
	"Armas",                             // weapons remain with you into the next area of play
	"Regeneración",                            // items rematerialize on the map
	"Regen. lejana",                             // place user's character at the farthest point from other players
	"Permitir salida",                            // allow players to exit the level wihtout being killed first
	"Mismo mapa",                                 // use the same map as last time
	"Forzar regen.",                            // make the respawn command happen now.
	"Daños al caer",                           // players take damage from falling down
	"Potenciadores inst.",                            // make instant power-ups available in the game
	"Potenciadores",                             // allow power-ups to be used in the game
	"Botiquines",                             // allow health kits to be used in the game
	"Permitir blindaje",                              // allow armor to be used in the game
	"Munición infinita",                            // unlimited ammunition
	"Campo visual fijo",                                // limits the user's field-of-vision
	"Juego en equipo",                                 // team-play
	"Herir/compañeros",                           // hurt teammates allows players to cause damage to team-mates.
	"Cambio rápido",                              // allows a player to change weapons faster than normal
	"Pasos",                                // footsteps become audible or silent
	"Permitir gancho",                               // allow hook
	"Ver casquillos",                           // show shotgun shells?
	
	"Dirección IP del servidor",                        // server internet provider address
	"No se han encontrado partidas locales",                     // no local games found TX NOTE:sentence.
	"Iniciando mapa",                             // starting map
	"Nombre/servidor",                              // server name
	"Nombre/jugador",                              // player name
	"Nombre/equipo",                                // team name
	
	"¿Hacerse el harakiri?",                        // confirm quit TX NOTE:Do not translate Harakiri--it's Jpn.
	
	"Sí",                             // 'yes' response
	"No",                             // 'no' response

	"Guardar partida",								// cek[1-3-00] changed from 'save' to 'save game'
	"Cargar",								//cek[12-6-99]
	"Guardar",								//cek[12-6-99]
	"Eliminar",						 	//cek[12-6-99]
	"Selecciona archivo de configuración",			//cek[12-6-99]
	"¿Eliminar la selección?",						//cek[12-6-99]
												
	"Cargar",								//cek[12-6-99]
	"Armad.",									//cek[12-6-99]
	"Salud",									//cek[12-6-99]
	"Nivel",									//cek[12-6-99]
	"Monstruos",									//cek[12-6-99]
	"Secretos",									//cek[12-6-99]
	"Tiempo",										//cek[12-6-99]
	"Total",									//cek[12-6-99]
	"Episodio",									//cek[12-6-99]

	"Usar sonido tridimensional",				//cek[12-6-99]

	"Guardado",									//cek[12-7-99]
	"Cargado",									//cek[12-7-99]
	"Eliminado",									//cek[12-7-99]

	"Suavizar el ratón",								//cek[12-7-99]
	"Rueda arriba",									//cek[12-7-99]
	"Rueda abajo",								//cek[12-7-99]

	"No permitir ALT+TAB",						//cek[12-7-99]
	"Sangre y carne",						//cek[12-7-99]
	"Balanceo del arma",							//cek[12-7-99]
	"Cambio auto de arma",						//cek[12-7-99]

	"Intensidad",								//cek[12-7-99] cek[12-13-99]
	"Armas brillantes",							//cek[12-7-99]
	"Agujeros de bala",								// SCG[1/4/00]: 

	"Adelante",									//cek[12-9-99]
	"Lateral",										//cek[12-9-99]
	"Arriba",										//cek[12-9-99]
	"Inclinación",									//cek[12-9-99]
	"Desviación",										//cek[12-9-99]
	"Usar mando",								//cek[12-9-99]
	"Configurar eje",							//cek[12-9-99]
	"Configurar las opciones del mando",				//cek[12-9-99]
	"Seleccionar acción para este botón",			//cek[12-9-99]
	"Pulsar botón del joystick para configurar",		//cek[12-9-99]
	"Límite",								//cek[12-9-99]
	"Sensibilidad",								//cek[12-9-99]

	"Capturar la bandera",							//cek[12-10-99]
	"Vuelta",										//cek[12-10-99]
	"56.6k",									//cek[12-10-99]
	"Auto",										//cek[12-10-99]
	"Equipo 1",									//cek[12-10-99]
	"Equipo 2",									//cek[12-10-99]

	"Ver introducción",								//cek[12-13-99]
	"Sist. habilidad",							//cek[12-13-99]
	"Velocidad del ratón",								//cek[12-13-99]

	"\"¡COGE!\"",								// cek[1-3-00]
	"\"¡VEN!\"",							// cek[1-3-00]
	"\"¡ESPERA!\"",							// cek[1-3-00]
	"\"¡ATACA!\"",							// cek[1-3-00]
	"\"¡RETÍRATE!\"",							// cek[1-3-00]

	"Selec. arma %d",							// cek[1-4-00]  as in "Select Weapon 1" (leave the %d)

	"HUD cíclico",								// cek[1-5-00]  -- for these, translate HUD to something short and reasonably
	"Siguiente HUD",									// cek[1-5-00]  -- similar in meaning.
	"HUD anterior",									// cek[1-5-00]
	"Usar HUD",									// cek[1-5-00]

	"Hablar",										// cek[1-11-00]
	"Puntuación",								// SCG[1/13/00]: 
	"Hablar al equipo",								// cek[1-22-00]
	"%s ha sido asignada a %s",				// cek[1-22-00]  as in "F is already assigned to attack" (leave in %s)
	"¿Reemplazar?",									// cek[1-22-00]

	"Entrar",								// cek[2-1-00]
	"Comunidad DK",								// cek[2-1-00]
	"¿Continuar?",								// cek[2-1-00]

	// for the next two, all \n in the strings denotes a string break.  These must remain in place and the translated
	// string should have them in roughly the same place.
	"Si continuas saldrás de Daikatana e iniciarás\nel explorador del servidor Multijugador.",	// cek[2-1-00]
	"Para noticias, chats, foros y clasificaciones\nve a www.daikatananews.com.\nSi continúas iniciarás tu explorador de red.",			// cek[2-1-00]

	"Arma %d",									// cek[3-6-00]: treat as "weapon 1"
	"Cancelan",									// cek[3-9-00]					
	"www.planetdaikatana.com",													// cek[3-9-00]
	"Ir a la www.planetdaikatana.com.\nSi continúas iniciarás tu explorador de red.",			// cek[3-9-00]
};


static char *tongue_menu_options[] = 
{
	"Correr siempre",                               // makes user's characters run everywhere
	"Mirar y saltar",                               // look + jump
	"Mirar y disparar",                               // look + strafe (shoot wildly)
	"Fijación automática de objetivos",                           // computer aims at nearest target for the user
	"Punto de mira",                                // a crosshair is displayed 
	"Arma visible",                           // weapon is shown
	// ************************************************
	"Subtítulos",                                // text subtitles
	// ************************************************
	
	"Volumen de los efectos de sonido",                      // volume for sound fx
	"Volumen de la música",                             // volume for music
	"Volumen de las películas",					// volume for movies
	
	"Software",                                 // video mode, software
	"OpenGL",                                   // video mode, opengl
	"Aplicar cambios",                            // apply video mode changes TX NOTE:sentence.
	
	"Pantalla completa",                               // make view use the full screen
//		"Stipple Alpha",                            // stipple alpha TX NOTE:this is a type of speed optimization technique. Don't translate.
	"Sombras",                                  // allow shadows
	"Controlador",                                   // video driver
	"Resolución",                               // choose/set screen display resolution
	"Tamaño de pantalla",                              // choose/set screen size
	"Brillo",                               // choose/set brightness
	"Calidad de textura",                          // choose/set quality of 3D image textures 
	"Barra de estado visible",                    // choose/set whether the status bar is visible.
	// ************************************************
	"Detalles de niebla y agua",							// SCG[12/4/99]: 
	"Asaltante",									// difficulty level easy
	"Samurai",									// difficulty level medium
	"Pistolero",									// difficulty level hard

	"Lluvia y nieve",							// cek[1-11-00]
	"Niebla"										// cek[1-11-00]	
		// ************************************************
		
};



//descriptions for the texture quality settings.
static char *texture_quality_desc[] =
{
	"Toda",
	"1/2",
	"1/4",
	"1/8"
};


// weapon names TX NOTE:"you picked up" +.TX NOTE:translate what you can. "localize" spellings of anything else.
static char *tongue_weapons[] =
{
	"Sin munición",		//only shows up if you have no ammo for the weapon you're trying to use. TX NOTE:treat like sentence "you have "no ammo".
	"la ballista",
	"proyectiles de ballista",
	"la ballesta",
	"flechas",
	"el C4 Vizatergo",
	"proyectiles de C4",
//		"el Celestril",
//		"la barrera de Celestril",
	"la daikatana de la daikatana",
//		"el destierro de la daikatana",
//		"la barrera de la daikatana",
//		"la hoja danzarina de la daikatana",
//		"la melee de la daikatana",
//		"la Posesión de la daikatana",
//		"el Próteo de la daikatana",
	"el disco de Dédalo",
	"el disco",
	"el guante disruptor",
//		"la linterna noruega",
	"la Automark 2020", //TX NOTE:a ficticious automatic pistol--US game currently reads "Glock", but that's a trademark.
	"balas",
	"el martillo",
	"el ionizador",
	"el proyectil ion",
	"la pistola cinética",
	"la bola de raíles",
	"el metamaser",
	"munición del metamaser",
//		"la mano de oro de Midas",
//		"masa de pirita",
	"la Pesadilla de Nharre",
	"la esfera de sacudidas",
	"el rayo Nova",
	"la esfera de sacudidas",
	"el láser",
	"granadas de cordita",
	"el generador de ondas",
	"la esfera de choque",
	"el ciclofusil",
	"cartuchos",
	"el sidewinder",
	"la esfera de sacudidas",
	"la garra de plata",
	"el slugger",
	"proyectiles de slugger",
	"el cetro de Stavros",
	"rocas de lava",
	"el Llameante",
	"llameantes",
//		"el gancho Tazer",
	"el tridente de Poseidón",
	"puntas de tridente",
	"el Envenenador",
	"la esfera de sacudidas",
	"el mechón de Wyndrax",
	"mechones",
	"la mano de Zeus",
	"el Rayo",
	"las manos gaseosas"

};

// armor TX NOTE:"you picked up" +.TX NOTE:translate what you can. "localize" spellings of anything else.
static char *tongue_armors[] =
{
	"el blindaje cerámico",
	"el blindaje cromático",
	"la armadura de plata",
	"a armadura de oro",
	"la cota de malla",
	"la armadura negra",
	"el blindaje de kevlar",
	"el blindaje de ebonita"
};


// monster names TX NOTE:NOMINITIVE CASE. translate what you can. "localize" spellings of anything else.
static char *tongue_monsters[] =
{
	"El robojabalí de batalla",
	"El prisionero negro",
	"El buboide",         // TX NOTE:must match script.
	"La robocámara",
	"El centurión",
	"The Cerberus",         // TX NOTE:deleted. don't translate.
	"El ametrallador volante",
	"La columna cariátide",
	"El robocaimán",
//		"Los cíclopes",
	"La esfera asesina",
	"El murciélago asesino",
	"El dragón",         // TX NOTE:must match script.
	"El enano",
	"La pandillera",
	"El barquero",
	"El arquero",
	"La roborrana",
	"El rey Garroth",
	"Los peces de colores",
	"El grifo",
	"La arpía",
	"El carcelero",
	"Kage",
	"King Minos",         // TX NOTE:deleted. don't translate.
	"El primer caballero",
	"El segundo caballero",
	"El mono de laboratorio",
	"El trabajador de laboratorio",
	"La robotorreta",
	"Licántropo",
	"Medusa",         // TX NOTE:must match script.
	"El Minotauro",
	"El guardia de Mishima",         // TX NOTE:must match script.
	"Nharre",              // TX NOTE:must match script  don't translate
	"La rata de la peste",
	"La rata de alcantarilla",
	"El cura",
	"El prisionero",
	"El roboprotópodo",
	"El mentálico",
	"El RageMaster 5000",
	"El pandillero lanzacohetes",
	"El PM lanzacohetes",
	"Los gusanos venenosos",
	"El sátiro",
	"El comando SEAL",         // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
//		"El hombre rana SEAL",            // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"El capitán SEAL",         // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"La chica SEAL",         // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"El tiburón",
	"El robomosquito",
	"El esqueleto",
	"El trabajador delgado",
	"El trabajador gordo",
	"El robodesatascador",
	"La pequeña araña",
	"La araña",
	"Stavros",         // TX NOTE:must match script.
	"El Cirujano",
	"El ladrón",
	"El robomoscardón",
//		"El roboartillero de raíles",
	"El pandillero de Uzis",
	"El prisionero blanco",
	"El Hechicero",         // TX NOTE:must match script.
	"Wyndrax",         // TX NOTE:must match script.
	"La gaviota",
	"La luciérnaga",
	"El espíritu",
	"Mikiko"									// SCG[12/5/99]: 
};

// monster names ACCUSATIVE CASE. NOT USED IN ENGLISH. translate what you can. "localize" spellings of anything else.
static char *tongue_monsters_acc[] =
{
	"al robojabalí de batalla",
	"al prisionero negro",
	"al buboide",         // TX NOTE:must match script.
	"a la robocámara",
	"al centurión",
	"the Cerberus",         // TX NOTE:deleted. don't translate.
	"al ametrallador volante",
	"a la columna cariátide",
	"al robocaimán",
//		"a los cíclopes",
	"a la esfera asesina",
	"al murciélago asesino",
	"al dragón",         // TX NOTE:must match script.
	"al enano",
	"a la pandillera",
	"al barquero",
	"al arquero",
	"a la roborrana",
	"al rey Garroth",
	"a los peces de colores",
	"al grifo",
	"a la arpía",
	"al carcelero",
	"a Kage",
	"King Minos",         // TX NOTE:deleted. don't translate.
	"al primer caballero",
	"al segundo caballero",
	"al mono de laboratorio",
	"al trabajador de laboratorio",
	"a la robotorreta",
	"al licántropo",
	"a Medusa",         // TX NOTE:must match script.
	"al Minotauro",
	"al guardia de Mishima",         // TX NOTE:must match script.
	"Nharre",              // TX NOTE:must match script  don't translate
	"a la rata de la peste",
	"a la rata de alcantarilla",
	"al cura",
	"al prisionero",
	"al roboprotópodo",
	"al mentálico",
	"al RageMaster 5000",
	"al pandillero lanzacohetes",
	"al PM lanzacohetes",
	"al gusano venenoso",
	"al sátiro",
	"al comando SEAL",         // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
//		"al hombre rana SEAL",            // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"al capitán SEAL",         // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"a la chica SEAL",         // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"al tiburón",
	"al robomosquito",
	"al esqueleto",
	"al trabajador delgado",
	"al trabajador gordo",
	"al robodesatascador",
	"a la pequeña araña",
	"a la araña",
	"a Stavros",         // TX NOTE:must match script.
	"al Cirujano",
	"al ladrón",
	"al robomoscardón",
//		"al roboartillero de raíles",
	"al pandillero de Uzis",
	"al prisionero blanco",
	"al Hechicero",         // TX NOTE:must match script.
	"a Wyndrax"         // TX NOTE:must match script.
	"la gaviota",
	"la luciérnaga",
	"el espíritu",
	"Mikiko"									// SCG[12/5/99]: 
};


static char *tongue_world[] =
{
	"todavía hay más",                                 // triggers to go until sequence complete TX NOTE:## +.
	"¡Secuencia completada",                         // trigger sequence completed TX NOTE:sentence.
	
	"Debes poseer",                              // need a specific key TX NOTE:followed by the key needed.
	"¡Debes poseer la llave correcta!\n",               // wrong key in possession TX NOTE:sentence.
		
	// ******************************************
	"Imposible abrir así\n",					       // Can't be opened this way
	// ******************************************
			
	"Sientes cómo el veneno abandona tu organismo\n",    // poison expiring TX NOTE:sentence.
	"Tus oxipulmones están casi vacíos\n",           // oxylung expiring TX NOTE:sentence.
	"Tu traje ambiental se está agotando\n",                // envirosuit expiring TX NOTE:sentence.
	"Tu megaescudo se está agotando\n",                 // megashield expiring TX NOTE:sentence.
	"La esfera espectral se ha desvanecido\n",                   // wraith orb expiring TX NOTE:sentence.
	"La mejora de potencia se está agotando\n",				  // power boost expiring TX NOTE:sentence.
	"La mejora de ataque se está agotando\n",                // attack boost expiring TX NOTE:sentence.
	"La mejora de velocidad se está agotando\n",                 // speed boost expiring TX NOTE:sentence.
	"La mejora de acrobacia se está agotando\n",             // jump boost expiring TX NOTE:sentence.
	"La mejora de vitalidad se está agotando\n",              // health boost expiring TX NOTE:sentence.
		
	"se ha unido a la partida",                            // player joined the game TX NOTE:NAME +.
	"ha abandonado el nivel",                           // player exited the level TX NOTE:NAME +.
	
	"Hiro Miyamoto",                              // Hiro Miyamoto's full name TX NOTE:same as in script.
	"Superfly Johnson",                           // Superfly's full name TX NOTE:same as in script.
	"Mikiko Ebihara",                             // Mikiko's full name TX NOTE:same as in script.
		
	"",
	"",
	"",
	"",
	"",
		
	//***********************************
	"Imposible salir del nivel sin\n",					// SCG[10/18/99]: no exiting level without sidekicks 
	"Imposible salir del nivel sin\n",						// SCG[11/9/99]: No exiting without a key
	"¡Envenenado!\n",								// NSS[11/29/99]: Needed a generic and yet global poisoned message
	"¡Has encontrado un secreto!\n",					// cek[1-3-00]
	//***********************************
			
	"La calavera mágica se está desvaneciendo.\n"                   // cek[2-9-00]
};

static char *tongue_deathmsg_daikatana[] = 
{
	"A %s le abrió en canal %s\n"
};

static char *tongue_deathmsg_self[] = 
{
	" se ha roto la crisma.\n",                          // death, fell TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"se ha ahogado.\n",                             // death, drowned TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"está doradito y crujiente.\n",                                // death, lava TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"ha sido babeado.\n",                                 // death, slimed TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"se ha envenenado.\n",                                // death, poison TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"no pudo sobrevivir.\n",                  // death, killed self TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"se ha teletransportado en cachitos.\n",                            // death, teleport TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"se ha espachurrado.\n",                            // death, crushed TX NOTE:character +. TX NOTE:use your own appropriate slang.
		
	"se ha comido un platito de C4.\n"                        // death, C4 explosives TX NOTE:character +. TX NOTE:use your own appropriate slang.
};


	// TX NOTE:use your own appropriate slang for all entries in this section.
	
	//TX NOTE:Grammar note: use constructions that put the victim in the accusative case (direct object)
	
static char *tongue_deathmsg_weapon1[] =
{
	"¡%s humilló a %s!",
	"¡%s le ha dado una buena a %s!",
	"¡%s ionizó a %s!",
	"¡%s dejó a %s hecho un asco!",
	"¡%s ha llenado de plomo a %s!",
	"¡%s mandó a %s a la luna!",
	"¡%s dejó tiritando a %s!"
};

static char *tongue_deathmsg_weapon2[] =
{
	"¡%s ha dejado a %s para el arrastre!",
	"¡%s se ha cepillado a %s!",
	"¡%s ha convertido a %s en cenizas!",
	"¡%s tenderized %s!",
	"¡%s vuela a %s!",
	"¡%s se lo tragado!"
};

static char *tongue_deathmsg_weapon3[] =
{
	"¡%s ha hecho trizas a %s!",
	"¡%s ha dejado a %s para el arrastre!",
	"¡%s se ha cepillado a %s!",
	"¡%s ha chamuscado a %s!",
	"¡%s ha frito a %s!",
	"¡%s ha liquidado a %s!"
};

static char *tongue_deathmsg_weapon4[] =
{
	"¡%s le levantó la tapa a %s!",
	"¡%s ha pulverizado a %s!",
	"¡%s ha rajado a %s de arriba abajo!",
	"¡%s le ha dejado fatal a %s!",
	"¡%s ha congelado a %s!",
	"¡%s le ha hecho un agujero a %s!",
	"¡%s ha masacrado a %s!"
};

static char *tongue_scoreboard[] =
{
	"Nombre",
	"Muertes",
	"Tiempo",
	"de señal",
	"Disparos",
	"Blancos",
	"Muerto",
	"Score"										// cek[1-22-00]
};


static char *tongue_difficulty[] =  // difficulty settings for 'new game' menu
{
	"Selecciona la dificultad:",           // difficulty header TX NOTE:using karate skill levels.
	"Cinturón blanco",                   // newbie
	"Cinturón azul",                    // average
	"Cinturón negro"                    // expert (or so they think!)
};


static char *tongue_skincolors[] =
{
	"Azul",                         // blue
	"Verde",                        // green
	"Rojo",                          // red
	"Dorado"                          // gold
};


static char *tongue_statbar[] =
{
	"POTENCIA",                        // skill 'power' label
	"ATAQUE",                       // skill 'attack' label
	"VELOCIDAD",												// skill 'speed' label
	"ACROBACIA",													// skill 'acrobatic' label
	"VITALIDAD",											// skill 'vitality' label
		
		//	"CORAZA",                        // armor label
		//  "SALUD",												// health label
		//  "MUNICIÓNES",													// ammo label
		//  "MUERTES",												// kills label
		//  "NIVEL",  											// experience points label
		
		
		// not used in non-english versions
	"",																	 // armor label
	"",																	 // health label
	"",																	 // ammo label
	"",																	 // kills label
	"",																	 // experience points label
		
		
	"¡NIVEL ARRIBA!",                    // experience level up
	"Seleccionar",               // select skill
	"Añadir punto",            // add point to skill
		
		//*********************************************
	"CARGANDO..."                  // loading new level
		//*********************************************
};


static char *tongue_weapon_misc[] =
{
	"¡los módulos C4 están desestabilizados!\n"    // C4 modules are de-stabilizing
};


static char *tongue_sidekick[] =    // sidekick commands
{ 
	"Coge",		                        // 'get' item command
	"Sigue",                          // 'come here', 'come to me'
	"Quédate",                        // 'stay' command, 'don't move'
	"Ataca",                          // 'attack' command
	"No ataques",                     // 'back off', 'move away'
		
	// *********************************************
	"Puedo coger"                    //  asking to pick up an item: "Can I have the shotgun?"
	// *********************************************
};

static char *tongue_ctf[] =
{
	"Bandera roja",
	"Bandera azul",
	"Equipo desconocido",

	"Bandera %s",											// when translating, treat '%s Flag' as 'red Flag' and place the %s before or after flag
														// depending on the syntax of the language.  ie if the expression should be 'Flag red' use 'Flag %s'
	"Rojo",
	"Azul",
	"Cromo",
	"Metálico",
	"Verde",
	"Naranja",
	"Púrpura",
	"Amarillo",

	"Tu puntuación es: %s:%d, %s:%d\n",
	"¡Has capturado la %s!\n",
	"¡%s de tu equipo ha capturado la %s!\n",
	"¡%s del Equipo %s ha capturado tu bandera!\n",		// when translating, treat '%s team' as 'red team' and place the %s before or after team
															// depending on the syntax of the language.  ie if the expression should be 'team red' use 'team %s'
	"Has cogido la %s\n",
	"%s de tu equipo ha cogido la %s\n",
	"%s ha robado tu bandera\n",
	"La %s hasido devuelta\n",

	"%d es un número de equipo no válido.  Usa 1 o 2\n",
	"Estás en el equipo %d (%s)\n",
	"Ya estás en el equipo %d\n",
	"%s se ha unido al Equipo %s\n",									// translate same as with the %s team above

	"¡El Equipo %s ha ganado!\nPuntuación final: %s:%d, %s:%d\n",		// translate same as with the %s team above
	"¡La partida termina en empate!\n",
	"¡Ha terminado el tiempo!\n",
	"¡Se ha alcanzado el límite de capturas!\n"
};

static char *tongue_deathtag[] =
{
	"Bomba",														// as in backpack
	"Bomba %s",													// as in red pack
	"¡Se ha alcanzado el límite de puntuación!\n",
	"¡Se te está acabando el tiempo!\n",
	"¡Se te ha terminado el tiempo!\n",

	"¡%s del Equipo %s ha cogido la %s!\n",				// translate using the '%s team' guidelines above

	"¡Tu equipo consigue un punto!\n",
	"¡Tu equipo consigue %d puntos!\n",								// you get 2 points
	"¡El Equipo %s consigue un punto!\n",								// translate using the '%s team' guidelines above
	"¡El Equipo %s consigue %d puntos!\n",							// The red team gets 2 points

	"¡Has conseguido puntos!\n",
	"¡%s de tu equipo ha conseguido puntos!\n",
	"¡%s del equipo %s ha conseguido puntos!\n"
};
