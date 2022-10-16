#ifndef _WyndraxandWisp
#define _WyndraxandWisp

#define MAX_WISPS				10
#define MAX_WYNDRAX_WISPS		6


//WISP STATES
#define COMMAND_MASK			0xF000
#define STRIP_MASK				0x0FFF
#define WANDER					0x0000
#define COLLECT					0x0001
#define SPAWN					0x0020
#define DEAD					0x0400
#define NEWGOAL					0x8000

//WISP MASTER STATES
#define WAITING					0x0000
#define SENDING					0x0001

/* ***************************** Extern Vars *************************** */

extern float sin_tbl[];
extern float cos_tbl[];

/* ***************************** Local Structs *************************** */
typedef struct
{
	int sinofs;									// sine count offset
	int cosofs;									// cosine count offset
	int ToORFrom;								//Flag to tell me which way they should be going(to origin or away)
	int AlphaUpDown;							//Toggle Alpha blending up and down
	int Landing;								//Fly has decided to land on a FUCKING SOLID SURFACE!
	int AlphaBlending;							//Alpha Blending Personality number calculated from personality
	int AlphaCount;								//Alpha Blending Counter
	int mode;									//Current mode for wisp

	float Personality;							//Personality factor
	float SpawnTime;								//Time when wisp should spawn back
	CVector Dir;								//Direction to be going in
	CVector Last_Origin;						//Our Last Origin
	CVector Goal;								//Goal Origin

} wispHook_t;

typedef struct
{
    int NumFlies;								// # of wisps to spawn and for Wyndrax to suck down.
	int Max;									// Maximum distance for firefly to fly to
	int Velocity;								// Velocity for the FireFly
	int Mode;									// Mode for Master Wisp
	int CurrentWisp;							// Index to current Wisp being collected
	int ActiveWisps;							//Number of Wisps active in this cluster
	float Scale;								// Scale for model
	float Delta_Alpha;							// Frequency of Alpha change
	float Alpha_Level;							// Initial Alpha Level
	char Model[100];							// Model/Sprite to use.
	CVector NewGoal;							// Wyndrax's Position
    userEntity_t *FireFlies[MAX_WISPS];			// list of firefly ents
	
} wispMasterHook_t;

#endif