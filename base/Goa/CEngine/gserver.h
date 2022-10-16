/******
gserver.h
GameSpy C Engine SDK
  
Copyright 1999 GameSpy Industries, Inc

Suite E-204
2900 Bristol Street
Costa Mesa, CA 92626
(714)549-7689
Fax(714)549-0757

******

 Please see the GameSpy C Engine SDK documentation for more 
 information

******/

#include "goaceng.h"
#include "hashtable.h"
struct GServerImplementation
{
	char ip[16];
	int port;
	int ping;
	HashTable keyvals;

};

typedef struct
{
	char *key, *value;
} GKeyValuePair;

typedef struct 
{
	KeyEnumFn EnumFn;
	void *instance;
} GEnumData;

void ServerFree(void *elem);
GServer ServerNew(char *ip, int port);
void ServerParseKeyVals(GServer server, char *keyvals);


