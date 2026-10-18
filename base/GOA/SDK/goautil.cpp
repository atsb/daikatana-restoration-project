/******
goautil.c
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

 Updated 6/9/99 - DDW
  Added get_sockaddrin function, and use for resolving
  Made portable with additions from CEngine code
  Double check that we don't bind to localhost (instead use INADDR_ANY)

******/

/********
INCLUDES
********/
#include "../../server/server.h"
#include "goautil.h"
#include <stdlib.h>
#include <stdio.h>
#include "../nonport.h"
#include <SDL3_net/SDL_net.h>
#include <string.h>
#include <assert.h>

extern cvar_t	*public_server;			// should heartbeats be sent
#ifdef __cplusplus
extern "C" {
#endif

/********
DEFINES
********/
#define MASTER_PORT 27900
#define MASTER_ADDR "master.gamespy.com"
#define HB_TIME 300000 //5 minutes
#define MAX_DATA_SIZE 1400
#define INBUF_LEN 256
#define BUF_SIZE 1400

/********
GLOBAL VARS
********/
goa_querycallback_t goa_basic_callback = NULL;
goa_querycallback_t goa_info_callback = NULL;
goa_querycallback_t goa_rules_callback = NULL;
goa_querycallback_t goa_players_callback = NULL;
char goa_secret_key[256];

/********
TYPEDEFS
********/
typedef unsigned char uchar;

typedef enum {qtunknown, qtbasic, qtinfo, qtrules, qtplayers, qtstatus, qtpackets, qtecho, qtsecure} query_t;


/********
VARS
********/
static const char *queries[]={"","basic","info","rules","players","status","packets", "echo", "secure"};

static unsigned long lastheartbeat = 0;
static int queryid = 1;
static int packetnumber;
static int qport;
static void *udata = NULL;
static char gname[24];

static NET_DatagramSocket *querysock = NULL;
static NET_DatagramSocket *hbsock = NULL;
static NET_Address *master_address = NULL;


/********
PROTOTYPES
********/
static void send_heartbeat(int statechanged);
static void parse_query(char *query, NET_Address *sender, Uint16 sender_port);


/****************************************************************************/
/* PUBLIC FUNCTIONS */
/****************************************************************************/

/* goa_init: Initializes the sockets, etc. Returns an error value
if an error occured, or 0 otherwise */
int goa_init(int queryport, char *gamename, char *ip, void *userdata)
{
    SDL_PropertiesID props;
    NET_Address *bind_address = NULL;

    qport = queryport;
    strncpy(gname, gamename, 23);
    gname[23] = '\0';
    udata = userdata;

    props = SDL_CreateProperties();
    if (!props)
        return E_GOA_WSOCKERROR;
    SDL_SetBooleanProperty(props, NET_PROP_DATAGRAM_SOCKET_REUSEADDR_BOOLEAN, true);

    if (ip && ip[0] && strcasecmp(ip, "localhost") != 0)
    {
        bind_address = NET_ResolveHostname(ip);
        if (!bind_address)
        {
            SDL_DestroyProperties(props);
            return E_GOA_BINDERROR;
        }
        if (NET_WaitUntilResolved(bind_address, -1) < 0)
        {
            NET_UnrefAddress(bind_address);
            SDL_DestroyProperties(props);
            return E_GOA_BINDERROR;
        }
    }

    querysock = NET_CreateDatagramSocket(bind_address, (Uint16)queryport, props);
    if (bind_address)
        NET_UnrefAddress(bind_address);
    SDL_DestroyProperties(props);
    if (!querysock)
        return E_GOA_BINDERROR;

    master_address = NET_ResolveHostname(MASTER_ADDR);
    if (!master_address)
    {
        NET_DestroyDatagramSocket(querysock);
        querysock = NULL;
        return E_GOA_CONNERROR;
    }
    if (NET_WaitUntilResolved(master_address, -1) < 0)
    {
        NET_UnrefAddress(master_address);
        master_address = NULL;
        NET_DestroyDatagramSocket(querysock);
        querysock = NULL;
        return E_GOA_CONNERROR;
    }

    hbsock = NET_CreateDatagramSocket(NULL, 0, 0);
    if (!hbsock)
    {
        NET_UnrefAddress(master_address);
        master_address = NULL;
        NET_DestroyDatagramSocket(querysock);
        querysock = NULL;
        return E_GOA_WSOCKERROR;
    }

    return 0;
}

/* goa_process_queries: Processes any waiting queries, and sends a
heartbeat if 5 minutes have elapsed */
void goa_process_queries(void)
{
    static char indata[INBUF_LEN];
    unsigned int tc = current_time();

    if (!querysock || !hbsock || !master_address)
        return;

    if (tc - lastheartbeat > HB_TIME)
        send_heartbeat(0);

    for (;;)
    {
        NET_Datagram *datagram = NULL;
        int error;

        if (!NET_ReceiveDatagram(querysock, &datagram) || !datagram)
            return;
        error = datagram->buflen;
        if (error >= INBUF_LEN)
        {
            NET_DestroyDatagram(datagram);
            continue;
        }

        memcpy(indata, datagram->buf, error);
        indata[error] = '\0';
        if (datagram->addr)
            parse_query(indata, datagram->addr, datagram->port);
        NET_DestroyDatagram(datagram);
    }
}

/* goa_send_statechanged: Sends a statechanged heartbeat, call when
your gamemode changes */
void goa_send_statechanged(void)
{
    send_heartbeat(1);
}

/* goa_shutdown: Cleans up the sockets and shuts down */
void goa_shutdown(void)
{
    if (querysock)
    {
        NET_DestroyDatagramSocket(querysock);
        querysock = NULL;
    }

    if (hbsock)
    {
        NET_DestroyDatagramSocket(hbsock);
        hbsock = NULL;
    }

    if (master_address)
    {
        NET_UnrefAddress(master_address);
        master_address = NULL;
    }
}

/****************************************************************************/

/* value_for_key: this returns a value for a certain key in s, where s is a string
containing key\value pairs. If the key does not exist, it returns  NULL. 
Note: the value is stored in a common buffer. If you want to keep it, make a copy! */
char *value_for_key(const char *s, const char *key)
{
	static int valueindex;
	const char *pos;
	char *pos2;
	char keyspec[256]="\\";
	static char value[2][256];

	valueindex ^= 1;
	strcat(keyspec,key);
	strcat(keyspec,"\\");
	pos = strstr(s,keyspec);
	if (!pos)
		return NULL;
	pos += strlen(keyspec);
	pos2 = value[valueindex];
	while (*pos && *pos != '\\')
		*pos2++ = *pos++;
	*pos2 = '\0';
	return value[valueindex];
}

/*****************************************************************************/
/* Various encryption / encoding routines */

void swap_byte ( uchar *a, uchar *b )
{
	uchar swapByte; 
	
	swapByte = *a; 
	*a = *b;      
	*b = swapByte;
}

uchar encode_ct ( uchar c )
{
	if (c <  26) return ('A'+c);
	if (c <  52) return ('a'+c-26);
	if (c <  62) return ('0'+c-52);
	if (c == 62) return ('+');
	if (c == 63) return ('/');
	
	return 0;
}

void gs_encode ( uchar *ins, int size, uchar *result )
{
	int    i,pos;
	uchar  trip[3];
	uchar  kwart[4];
	
	i=0;
	while (i < size)
	{
		for (pos=0 ; pos <= 2 ; pos++, i++)
			if (i < size) trip[pos] = *ins++;
			else trip[pos] = '\0';
			kwart[0] =   (trip[0])       >> 2;
			kwart[1] = (((trip[0]) &  3) << 4) + ((trip[1]) >> 4);
			kwart[2] = (((trip[1]) & 15) << 2) + ((trip[2]) >> 6);
			kwart[3] =   (trip[2]) & 63;
			for (pos=0; pos <= 3; pos++) *result++ = encode_ct(kwart[pos]);
	}
	*result='\0';
}

void gs_encrypt ( uchar *key, int key_len, uchar *buffer_ptr, int buffer_len )
{ 
	short counter;     
	uchar x, y, xorIndex;
	uchar state[256];       
	
	for ( counter = 0; counter < 256; counter++) state[counter] = (uchar) counter;
	
	x = 0; y = 0;
	for ( counter = 0; counter < 256; counter++)
	{
		y = (key[x] + state[counter] + y) & 255;
		x = (x + 1) % key_len;
		swap_byte ( &state[counter], &state[y] );
	}
	
	x = 0; y = 0;
	for ( counter = 0; counter < buffer_len; counter ++)
	{
		x = (x + buffer_ptr[counter] + 1)& 255;
		y = (state[x] + y) & 255;
		swap_byte ( &state[x], &state[y] );
		xorIndex = (state[x] + state[y])& 255;
		buffer_ptr[counter] ^= state[xorIndex];
	}
}
/*****************************************************************************/


/* packet_send: sends a key\value packet. Appends the queryid
key\value pair. Clears the buffer */
static void packet_send(NET_Address *addr, Uint16 port, char *buffer)
{
    char keyvalue[80];
    if (strlen(buffer) == 0)
        return;
    packetnumber++;
    Com_sprintf(keyvalue, sizeof(keyvalue), "\\queryid\\%d.%d", queryid, packetnumber);
    strcat(buffer, keyvalue);

    NET_SendDatagram(querysock, addr, port, buffer, (int)strlen(buffer));
    buffer[0] = '\0';
}

static void buffer_send(NET_Address *sender, Uint16 sender_port, char *buffer, char *newdata)
{
    char *pos, *lastkey;
    int bcount = 0;

    if (strlen(buffer) + strlen(newdata) < MAX_DATA_SIZE - 50)
    {
        strcat(buffer, newdata);
    }
    else
    {
        if (strlen(newdata) > MAX_DATA_SIZE - 50)
        {
            lastkey = pos = newdata;
            while (pos - newdata < MAX_DATA_SIZE - 50)
            {
                if ('\\' == *pos)
                {
                    if (bcount & 1 == 0)
                        lastkey = pos;
                    bcount++;
                }
                pos++;
            }
            if (lastkey == newdata)
                return;
            *lastkey = '\0';
            buffer_send(sender, sender_port, buffer, newdata);
            *lastkey = '\\';
            buffer_send(sender, sender_port, buffer, lastkey);
        }
        else
        {
            packet_send(sender, sender_port, buffer);
            strcpy(buffer, newdata);
        }
    }
}

static void send_basic(NET_Address *sender, Uint16 sender_port, char *outbuf)
{
    char keyvalue[BUF_SIZE] = "";
    assert(goa_basic_callback);
    goa_basic_callback(keyvalue, sizeof(keyvalue), udata);
    buffer_send(sender, sender_port, outbuf, keyvalue);
}

static void send_info(NET_Address *sender, Uint16 sender_port, char *outbuf)
{
    char keyvalue[BUF_SIZE] = "";
    assert(goa_info_callback);
    goa_info_callback(keyvalue, sizeof(keyvalue), udata);
    buffer_send(sender, sender_port, outbuf, keyvalue);
}

static void send_rules(NET_Address *sender, Uint16 sender_port, char *outbuf)
{
    char keyvalue[BUF_SIZE] = "";
    assert(goa_rules_callback);
    goa_rules_callback(keyvalue, sizeof(keyvalue), udata);
    buffer_send(sender, sender_port, outbuf, keyvalue);
}

static void send_players(NET_Address *sender, Uint16 sender_port, char *outbuf)
{
    char keyvalue[BUF_SIZE] = "";
    assert(goa_players_callback);
    goa_players_callback(keyvalue, sizeof(keyvalue), udata);
    buffer_send(sender, sender_port, outbuf, keyvalue);
}

static void send_echo(NET_Address *sender, Uint16 sender_port, char *outbuf, char *echostr)
{
    char keyvalue[MAX_DATA_SIZE];
    if (strlen(echostr) > MAX_DATA_SIZE - 50)
        return;
    Com_sprintf(keyvalue, sizeof(keyvalue), "\\echo\\%s", echostr);
    buffer_send(sender, sender_port, outbuf, keyvalue);
}

static void send_final(NET_Address *sender, Uint16 sender_port, char *outbuf, char *validation)
{
    char keyvalue[256];
    char encrypted_val[128];
    char encoded_val[200];
    int keylen;

    if (validation[0])
    {
        keylen = strlen(validation);
        if (keylen > 128)
            return;
        strcpy(encrypted_val, validation);
        gs_encrypt((unsigned char *)goa_secret_key, strlen(goa_secret_key), (unsigned char *)encrypted_val, keylen);
        gs_encode((unsigned char *)encrypted_val, keylen, (unsigned char *)encoded_val);
        Com_sprintf(keyvalue, sizeof(keyvalue), "\\validate\\%s", encoded_val);
        buffer_send(sender, sender_port, outbuf, keyvalue);
    }

    Com_sprintf(keyvalue, sizeof(keyvalue), "\\final\\");
    buffer_send(sender, sender_port, outbuf, keyvalue);
    packet_send(sender, sender_port, outbuf);
}

static void parse_query(char *query, NET_Address *sender, Uint16 sender_port)
{
    query_t querytype;
    char buffer[MAX_DATA_SIZE] = "";
    char *value;
    char validation[256] = "";

    queryid++;
    packetnumber = 0;

    for (querytype = qtbasic; querytype <= qtsecure; querytype = (query_t)((int)querytype + 1))
    {
        if ((value = value_for_key(query, queries[querytype])))
        {
            switch (querytype)
            {
                case qtbasic:
                    send_basic(sender, sender_port, buffer);
                    break;
                case qtinfo:
                    send_info(sender, sender_port, buffer);
                    break;
                case qtrules:
                    send_rules(sender, sender_port, buffer);
                    break;
                case qtplayers:
                    send_players(sender, sender_port, buffer);
                    break;
                case qtstatus:
                    send_basic(sender, sender_port, buffer);
                    send_info(sender, sender_port, buffer);
                    send_rules(sender, sender_port, buffer);
                    send_players(sender, sender_port, buffer);
                    break;
                case qtpackets:
                    send_basic(sender, sender_port, buffer); packet_send(sender, sender_port, buffer);
                    send_info(sender, sender_port, buffer); packet_send(sender, sender_port, buffer);
                    send_rules(sender, sender_port, buffer); packet_send(sender, sender_port, buffer);
                    send_players(sender, sender_port, buffer);
                    break;
                case qtecho:
                    send_echo(sender, sender_port, buffer, value);
                    break;
                case qtsecure:
                    strcpy(validation, value);
                    break;
                case qtunknown:
                    break;
            }
        }
    }
    send_final(sender, sender_port, buffer, validation);
}

/* send_heartbeat: Sends a heartbeat to the gamemaster,
adds \\statechanged\\ if statechanged != 0 */
static void send_heartbeat(int statechanged)
{
    char buf[256];
    if (!dedicated || !dedicated->value)
        return;
    if (!public_server->value)
        return;
    if (!hbsock || !master_address)
        return;

    Com_sprintf(buf, sizeof(buf), "\\heartbeat\\%d\\gamename\\%s", qport, gname);
    if (statechanged)
        strcat(buf, "\\statechanged\\");

    NET_SendDatagram(hbsock, master_address, MASTER_PORT, buf, (int)strlen(buf));
    lastheartbeat = current_time();
}

#ifdef __cplusplus
}
#endif
