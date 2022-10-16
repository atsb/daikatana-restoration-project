/******
goautil.h
GameSpy Developer SDK 
  
Copyright 1999 GameSpy Industries, Inc

Suite E-204
2900 Bristol Street
Costa Mesa, CA 92626
(714)549-7689
Fax(714)549-0757

******

 Please see the  GameSpy Developer SDK documentation for more 
 information

******/

#ifndef _GOAUTIL_H_
#define _GOAUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

/********
ERROR CONSTANTS
---------------
These constants are returned from goa_init to signal an error condition
***************/

#define E_GOA_WSOCKERROR	1
#define E_GOA_BINDERROR		2
#define E_GOA_DNSERROR		3
#define E_GOA_CONNERROR		4


/********
goa_querycallback_t
-------------------
This is the prototype for the callback functions your game needs to
implement for each of the four basic query types. The callback works the
same for each query type. 

[outbuf] is a pre-allocated buffer for you to place the query reply. It's size is
[maxlen] (default is 1400). If you need larger, you can adjust the 
	defines in goautil.c
[userdata] is the pointer that was passed into goa_init. You can use this for an
	object or structure pointer if needed.

Simply fill outbuf with the correct data for the query type (consult the sample
apps and the GameSpy Developer Spec). 
outbuf should be a NULL terminated ANSI string.
********/
typedef void (*goa_querycallback_t)(char *outbuf, int maxlen, void *userdata);	


/********
GLOBAL CALLBACK VARS
---------------------
Set these callbacks to your implementation of each callback function.
You can set these before or after calling goa_init, but they must be set
prior to the first call to goa_process_queries.
********/
extern goa_querycallback_t goa_basic_callback;
extern goa_querycallback_t goa_info_callback;
extern goa_querycallback_t goa_rules_callback;
extern goa_querycallback_t goa_players_callback;

/*********
SECRET KEY
----------
Initialize this string to the secret key to use for validation.
Make sure you null-terminate the key. 
You can set the key before or after calling goa_init, but it must be
set prior to the first call to goa_process_queries.
*********/
extern char goa_secret_key[256];


/************
GOA_INIT
--------
This creates/binds the sockets needed for heartbeats and queries/replies.
[queryport] is the port you want to accept queries on. Only 1 server per
	machine can use this port, so you need a way to specify alternate ports if
	running more than 1 server.
[gamename] is the unique gamename that you were given
[ip] is an optional parameter that determines which dotted IP address to bind to on
	a multi-homed machine. You can pass NULL to bind to all IP addresses.
[userdata] is an optional, implementation specific parameter that will be
	passed to all callback functions. Use it to store an object or structure
	pointer if needed.

Returns
0 is successful, otherwise one of the E_GOA constants above.
E_GOA_BINDERROR usually means that the queryport is already in use.
************/
int goa_init(int queryport, char *gamename, char *ip, void *userdata);


/*******************
GOA_PROCESS_QUERIES
-------------------
This function should be called somewhere in your main program loop to
process any pending server queries and send a heartbeat if 5 minutes has
elapsed.

Query replies are very latency sensative, so you should make sure this
function is called at least every 100ms while your game is in progress.
The function has very low overhead and should not cause any performance
problems.
********************/
void goa_process_queries(void);

/*****************
GOA_SEND_STATECHANGED
--------------------
This function forces a \statechanged\ heartbeat to be sent immediately.
Use it any time you have changed the gamestate of your game to signal the
master to update your status.
Also use it before your game exits by changing the gamestate to "exiting"
and sending a statechanged heartbeat.
*******************/
void goa_send_statechanged(void);

/*****************
GOA_SHUTDOWN
------------
This function closes the sockets created in goa_init and takes care of
any misc. cleanup. You should try to call it when before exiting the server
if goa_init was called.
******************/
void goa_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif