#include <SDL3/SDL.h>
#include <SDL3_net/SDL_net.h>
#include "qcommon.h"

#define MAX_LOOPBACK 4
#define MAX_NET_ADDRESS_CACHE 64

typedef struct
{
    byte data[MAX_MSGLEN];
    int datalen;
} loopmsg_t;

typedef struct
{
    loopmsg_t msgs[MAX_LOOPBACK];
    int get;
    int send;
} loopback_t;

cvar_t *net_shownet;
cvar_t *net_debug;
static cvar_t *noudp;

long out_bytes;
long in_bytes;
long last_time;

loopback_t loopbacks[2];
static NET_DatagramSocket *ip_sockets[2] = { NULL, NULL };

struct net_address_cache_entry_t
{
    qboolean used;
    byte ip[4];
    NET_Address *address;
};

static net_address_cache_entry_t net_address_cache[MAX_NET_ADDRESS_CACHE];
static NET_Address *net_broadcast_address;

static qboolean NET_ParseIPv4(const char *text, byte ip[4])
{
    unsigned int a, b, c, d;

    if (!text || sscanf(text, "%u.%u.%u.%u", &a, &b, &c, &d) != 4)
        return false;
    if (a > 255 || b > 255 || c > 255 || d > 255)
        return false;

    ip[0] = (byte)a;
    ip[1] = (byte)b;
    ip[2] = (byte)c;
    ip[3] = (byte)d;
    return true;
}

static NET_Address *NET_ResolveAddress(const char *host)
{
    NET_Address *address;
    NET_Status status;

    address = NET_ResolveHostname(host);
    if (!address)
        return NULL;

    status = NET_WaitUntilResolved(address, -1);
    if ((int)status < 0)
    {
        NET_UnrefAddress(address);
        return NULL;
    }

    return address;
}

static NET_Address *NET_CachedAddress(const byte ip[4])
{
    int i;

    for (i = 0; i < MAX_NET_ADDRESS_CACHE; i++)
    {
        if (net_address_cache[i].used &&
            memcmp(net_address_cache[i].ip, ip, 4) == 0)
            return net_address_cache[i].address;
    }

    return NULL;
}

static NET_Address *NET_CacheAddress(const byte ip[4])
{
    int i;
    NET_Address *address;
    char text[32];

    address = NET_CachedAddress(ip);
    if (address)
        return address;

    Com_sprintf(text, sizeof(text), "%u.%u.%u.%u",
        (unsigned int)ip[0], (unsigned int)ip[1],
        (unsigned int)ip[2], (unsigned int)ip[3]);

    address = NET_ResolveAddress(text);
    if (!address)
        return NULL;

    for (i = 0; i < MAX_NET_ADDRESS_CACHE; i++)
    {
        if (!net_address_cache[i].used)
        {
            net_address_cache[i].used = true;
            memcpy(net_address_cache[i].ip, ip, 4);
            net_address_cache[i].address = address;
            return address;
        }
    }

    NET_UnrefAddress(address);
    return NULL;
}

static NET_DatagramSocket *NET_CreateIPSocket(const char *net_interface, int port)
{
    SDL_PropertiesID props;
    NET_Address *bind_address = NULL;
    NET_DatagramSocket *socket;

    props = SDL_CreateProperties();
    if (!props)
    {
        Com_Printf("WARNING: SDL3_net: SDL_CreateProperties failed: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_SetBooleanProperty(props, NET_PROP_DATAGRAM_SOCKET_ALLOW_BROADCAST_BOOLEAN, true);
    SDL_SetBooleanProperty(props, NET_PROP_DATAGRAM_SOCKET_REUSEADDR_BOOLEAN, true);

    if (net_interface && net_interface[0] && stricmp(net_interface, "localhost") != 0)
    {
        bind_address = NET_ResolveAddress(net_interface);
        if (!bind_address)
        {
            SDL_DestroyProperties(props);
            Com_Printf("WARNING: SDL3_net: unable to resolve '%s': %s\n", net_interface, SDL_GetError());
            return NULL;
        }
    }

    socket = NET_CreateDatagramSocket(bind_address, (Uint16)(port == PORT_ANY ? 0 : port), props);

    if (bind_address)
        NET_UnrefAddress(bind_address);
    SDL_DestroyProperties(props);

    if (!socket)
        Com_Printf("WARNING: SDL3_net: unable to create UDP socket: %s\n", SDL_GetError());

    return socket;
}

static qboolean NET_GetLoopPacket(netsrc_t sock, netadr_t *net_from, sizebuf_t *net_message)
{
    int i;
    loopback_t *loop;

    loop = &loopbacks[sock];
    if (loop->send - loop->get > MAX_LOOPBACK)
        loop->get = loop->send - MAX_LOOPBACK;
    if (loop->get >= loop->send)
        return false;

    i = loop->get & (MAX_LOOPBACK - 1);
    loop->get++;
    memcpy(net_message->data, loop->msgs[i].data, loop->msgs[i].datalen);
    net_message->cursize = loop->msgs[i].datalen;
    memset(net_from, 0, sizeof(*net_from));
    net_from->type = NA_LOOPBACK;
    return true;
}

static void NET_SendLoopPacket(netsrc_t sock, int length, void *data)
{
    int i;
    loopback_t *loop;

    loop = &loopbacks[sock ^ 1];
    i = loop->send & (MAX_LOOPBACK - 1);
    loop->send++;
    memcpy(loop->msgs[i].data, data, length);
    loop->msgs[i].datalen = length;
}

static void NET_ClearAddressCache(void)
{
    int i;

    if (net_broadcast_address)
    {
        NET_UnrefAddress(net_broadcast_address);
        net_broadcast_address = NULL;
    }

    for (i = 0; i < MAX_NET_ADDRESS_CACHE; i++)
    {
        if (net_address_cache[i].used)
        {
            NET_UnrefAddress(net_address_cache[i].address);
            net_address_cache[i].address = NULL;
            net_address_cache[i].used = false;
        }
    }
}

void DKNET_Init(void)
{
    if (!NET_Init())
        Com_Error(ERR_FATAL, "SDL3_net initialization failed: %s", SDL_GetError());

    noudp = Cvar_Get("noudp", "0", CVAR_NOSET);
    net_shownet = Cvar_Get("net_shownet", "0", 0);
    net_debug = Cvar_Get("n_speeds", "0", 0);

    out_bytes = 0;
    in_bytes = 0;
    last_time = Sys_Milliseconds();
}

void DKNET_Shutdown(void)
{
    int i;

    for (i = 0; i < 2; i++)
    {
        if (ip_sockets[i])
        {
            NET_DestroyDatagramSocket(ip_sockets[i]);
            ip_sockets[i] = NULL;
        }
    }

    NET_ClearAddressCache();
    NET_Quit();
}

qboolean NET_CompareAdr(netadr_t a, netadr_t b)
{
    if (a.type != b.type)
        return false;

    if (a.type == NA_LOOPBACK)
        return true;

    if (a.type == NA_IP)
    {
        return a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] &&
               a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3] &&
               a.port == b.port;
    }

    return false;
}

qboolean NET_CompareBaseAdr(netadr_t a, netadr_t b)
{
    if (a.type != b.type)
        return false;
    if (a.type == NA_LOOPBACK)
        return true;

    if (a.type == NA_IP)
    {
        return a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] &&
               a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3];
    }

    return false;
}

qboolean NET_IsLocalAddress(netadr_t adr)
{
    return adr.type == NA_LOOPBACK;
}

char *NET_AdrToString(netadr_t a)
{
    static char s[64];

    if (a.type == NA_LOOPBACK)
        Com_sprintf(s, sizeof(s), "loopback");
    else if (a.type == NA_IP)
        Com_sprintf(s, sizeof(s), "%i.%i.%i.%i:%i",
            a.ip[0], a.ip[1], a.ip[2], a.ip[3],
            (int)SDL_Swap16BE(a.port));
    else
        Com_sprintf(s, sizeof(s), "unknown");

    return s;
}

qboolean NET_StringToAdr(char *s, netadr_t *a)
{
    char host[128];
    char *colon;
    int port = 0;
    byte ip[4];
    NET_Address *address;
    const char *resolved;

    memset(a, 0, sizeof(*a));

    if (!strcmp(s, "localhost"))
    {
        a->type = NA_LOOPBACK;
        return true;
    }

    strncpy(host, s, sizeof(host) - 1);
    host[sizeof(host) - 1] = '\0';

    colon = strrchr(host, ':');
    if (colon)
    {
        *colon++ = '\0';
        port = atoi(colon);
    }

    if (!host[0])
        return false;

    address = NET_ResolveAddress(host);
    if (!address)
        return false;

    resolved = NET_GetAddressString(address);
    if (!NET_ParseIPv4(resolved, ip))
    {
        NET_UnrefAddress(address);
        return false;
    }

    if (NET_CachedAddress(ip) == NULL)
    {
        int i;
        for (i = 0; i < MAX_NET_ADDRESS_CACHE; i++)
        {
            if (!net_address_cache[i].used)
            {
                net_address_cache[i].used = true;
                memcpy(net_address_cache[i].ip, ip, 4);
                net_address_cache[i].address = address;
                address = NULL;
                break;
            }
        }
    }

    if (address)
        NET_UnrefAddress(address);

    a->type = NA_IP;
    memcpy(a->ip, ip, 4);
    a->port = SDL_Swap16BE((Uint16)port);
    return true;
}

static void NET_OpenIP(void)
{
    cvar_t *ip;
    int port;
    int dedicated;

    ip = Cvar_Get("ip", "localhost", CVAR_NOSET);
    dedicated = Cvar_VariableValue("dedicated");

    if (!ip_sockets[NS_SERVER])
    {
        port = Cvar_Get("ip_hostport", "0", CVAR_NOSET)->value;
        if (!port)
        {
            port = Cvar_Get("hostport", "0", CVAR_NOSET)->value;
            if (!port)
                port = Cvar_Get("port", va("%i", PORT_SERVER), CVAR_NOSET)->value;
        }

        ip_sockets[NS_SERVER] = NET_CreateIPSocket(ip->string, port);
        if (!ip_sockets[NS_SERVER] && dedicated)
            Com_Error(ERR_FATAL, "Couldn't allocate dedicated server IP port");
    }

    if (dedicated)
        return;

    if (!ip_sockets[NS_CLIENT])
    {
        port = Cvar_Get("ip_clientport", "0", CVAR_NOSET)->value;
        if (!port)
        {
            port = Cvar_Get("clientport", va("%i", PORT_CLIENT), CVAR_NOSET)->value;
            if (!port)
                port = PORT_ANY;
        }

        ip_sockets[NS_CLIENT] = NET_CreateIPSocket(ip->string, port);
        if (!ip_sockets[NS_CLIENT])
            ip_sockets[NS_CLIENT] = NET_CreateIPSocket(ip->string, PORT_ANY);
    }
}

void NET_Config(qboolean multiplayer)
{
    int i;
    static qboolean old_config;

    if (old_config == multiplayer)
        return;

    old_config = multiplayer;

    if (!multiplayer)
    {
        for (i = 0; i < 2; i++)
        {
            if (ip_sockets[i])
            {
                NET_DestroyDatagramSocket(ip_sockets[i]);
                ip_sockets[i] = NULL;
            }
        }
        return;
    }

    if (!noudp->value)
        NET_OpenIP();
}

qboolean NET_GetPacket(netsrc_t sock, netadr_t *net_from, sizebuf_t *net_message)
{
    NET_Datagram *datagram = NULL;
    int ret;

    if (NET_GetLoopPacket(sock, net_from, net_message))
        return true;

    if (!ip_sockets[sock])
        return false;

    if (!NET_ReceiveDatagram(ip_sockets[sock], &datagram) || !datagram)
        return false;

    ret = datagram->buflen;
    if (ret >= (int)net_message->maxsize)
    {
        const char *from_string = datagram->addr ? NET_GetAddressString(datagram->addr) : "unknown";
        Com_Printf("Oversize packet from %s\n", from_string);
        NET_DestroyDatagram(datagram);
        return false;
    }

    memset(net_from, 0, sizeof(*net_from));
    net_from->type = NA_IP;
    if (!datagram->addr || !NET_ParseIPv4(NET_GetAddressString(datagram->addr), net_from->ip))
    {
        NET_DestroyDatagram(datagram);
        return false;
    }
    net_from->port = SDL_Swap16BE(datagram->port);

    memcpy(net_message->data, datagram->buf, ret);
    net_message->cursize = ret;
    in_bytes += ret;

    NET_DestroyDatagram(datagram);
    return true;
}

void NET_SendPacket(netsrc_t sock, int length, void *data, netadr_t to)
{
    NET_Address *address;

    if (net_debug->value)
    {
        out_bytes += length;

        {
            long diff = Sys_Milliseconds() - last_time;
            if (diff > 1000)
            {
                Com_Printf("IN: %d  OUT: %d\n", in_bytes / (diff / 1000), out_bytes / (diff / 1000));
                out_bytes = in_bytes = 0;
                last_time = Sys_Milliseconds();
            }
        }
    }

    if (to.type == NA_LOOPBACK)
    {
        NET_SendLoopPacket(sock, length, data);
        return;
    }

    if (to.type == NA_BROADCAST)
    {
        if (!ip_sockets[sock])
            return;

        if (!net_broadcast_address)
        {
            net_broadcast_address = NET_ResolveAddress("255.255.255.255");
            if (!net_broadcast_address)
                return;
        }
        address = net_broadcast_address;
    }
    else if (to.type == NA_IP)
    {
        if (!ip_sockets[sock])
            return;
        address = NET_CacheAddress(to.ip);
        if (!address)
            return;
    }
    else
    {
        Com_Error(ERR_FATAL, "NET_SendPacket: bad address type");
        return;
    }

    if (!NET_SendDatagram(ip_sockets[sock], address, SDL_Swap16BE(to.port), data, length))
    {
        if (dedicated->value)
            Com_Printf("NET_SendPacket ERROR: %s\n", SDL_GetError());
        else
            Com_DPrintf("NET_SendPacket ERROR: %s : %s\n", SDL_GetError(), NET_AdrToString(to));
    }
}
