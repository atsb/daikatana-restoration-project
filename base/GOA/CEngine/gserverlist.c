#include "goaceng.h"
#include "gserver.h"
#include "darray.h"
#include "../nonport.h"
#include "gutil.h"
#include <SDL3/SDL.h>
#include <SDL3_net/SDL_net.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MSHOST "master.gamespy.com"
#define MSPORT 28900
#define SERVER_GROWBY 32
#define LAN_SEARCH_TIME 3000
#define MASTER_CONNECT_TIMEOUT 10000
#define MASTER_SECURE_TIMEOUT 10000

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    int serverindex;
    unsigned long starttime;
} UpdateInfo;

struct GServerListImplementation
{
    GServerListState state;
    DArray servers;
    UpdateInfo *updatelist;
    char gamename[32];
    char seckey[32];
    char enginename[32];
    int maxupdates;
    int nextupdate;
    int abortupdate;
    ListCallBackFn CallBackFn;
    void *instance;
    char *sortkey;
    gbool sortascending;
    NET_StreamSocket *slsocket;
    NET_DatagramSocket *update_socket;
    unsigned long lanstarttime;
};

GServerList g_sortserverlist;

static NET_Address *GServerResolveAddress(const char *host)
{
    NET_Address *address;

    if (!host || !host[0])
        return NULL;

    address = NET_ResolveHostname(host);
    if (!address)
        return NULL;

    if (NET_WaitUntilResolved(address, -1) < 0)
    {
        NET_UnrefAddress(address);
        return NULL;
    }

    return address;
}

static void GServerDestroyStream(GServerList serverlist)
{
    if (serverlist->slsocket)
    {
        NET_DestroyStreamSocket(serverlist->slsocket);
        serverlist->slsocket = NULL;
    }
}

/* ServerListNew
----------------
Creates and returns a new (empty) GServerList. */
GServerList ServerListNew(char *gamename, char *enginename, char *seckey, int maxconcupdates, void *CallBackFn, int CallBackFnType, void *instance)
{
    GServerList list;

    (void)CallBackFnType;
    list = (GServerList)malloc(sizeof(struct GServerListImplementation));
    assert(list != NULL);
    memset(list, 0, sizeof(*list));

    list->state = sl_idle;
    list->servers = ArrayNew(sizeof(GServer), SERVER_GROWBY, ServerFree);
    list->maxupdates = maxconcupdates;
    list->updatelist = (UpdateInfo *)calloc(maxconcupdates, sizeof(UpdateInfo));
    assert(list->updatelist != NULL);
    strcpy(list->gamename, gamename);
    strcpy(list->seckey, seckey);
    strcpy(list->enginename, enginename);
    list->CallBackFn = CallBackFn;
    assert(CallBackFn != NULL);
    list->instance = instance;
    list->sortkey = "";
    return list;
}

/* ServerListFree
-----------------
Free a GServerList and all internal structures and servers. */
void ServerListFree(GServerList serverlist)
{
    int i;

    if (!serverlist)
        return;

    if (serverlist->slsocket)
        GServerDestroyStream(serverlist);

    if (serverlist->update_socket)
    {
        NET_DestroyDatagramSocket(serverlist->update_socket);
        serverlist->update_socket = NULL;
    }

    if (serverlist->updatelist)
    {
        for (i = 0; i < serverlist->maxupdates; i++)
            serverlist->updatelist[i].serverindex = -1;
    }

    ArrayFree(serverlist->servers);
    serverlist->servers = NULL;
    free(serverlist->updatelist);
    serverlist->updatelist = NULL;
    free(serverlist);
}


static GError InitUpdateList(GServerList serverlist)
{
    SDL_PropertiesID props;
    int i;

    props = SDL_CreateProperties();
    if (!props)
        return GE_NOSOCKET;

    SDL_SetBooleanProperty(props, NET_PROP_DATAGRAM_SOCKET_ALLOW_BROADCAST_BOOLEAN, true);
    SDL_SetBooleanProperty(props, NET_PROP_DATAGRAM_SOCKET_REUSEADDR_BOOLEAN, true);
    serverlist->update_socket = NET_CreateDatagramSocket(NULL, 0, props);
    SDL_DestroyProperties(props);

    if (!serverlist->update_socket)
        return GE_NOSOCKET;

    for (i = 0; i < serverlist->maxupdates; i++)
    {
        serverlist->updatelist[i].serverindex = -1;
        serverlist->updatelist[i].starttime = 0;
    }

    return GE_NOERROR;
}

static GError FreeUpdateList(GServerList serverlist)
{
    if (serverlist->update_socket)
    {
        NET_DestroyDatagramSocket(serverlist->update_socket);
        serverlist->update_socket = NULL;
    }
    return GE_NOERROR;
}

/* Create and connect a server list socket. */
static GError CreateServerListSocket(GServerList serverlist)
{
    NET_Address *master_address;

    master_address = GServerResolveAddress(MSHOST);
    if (!master_address)
        return GE_NODNS;

    serverlist->slsocket = NET_CreateClient(master_address, MSPORT, 0);
    NET_UnrefAddress(master_address);
    if (!serverlist->slsocket)
        return GE_NOSOCKET;

    if (NET_WaitUntilConnected(serverlist->slsocket, MASTER_CONNECT_TIMEOUT) < 0)
    {
        GServerDestroyStream(serverlist);
        return GE_NOCONNECT;
    }

    return GE_NOERROR;
}

/* Trigger the callback and set the new mode. */
static void ServerListModeChange(GServerList serverlist, GServerListState newstate)
{
    serverlist->state = newstate;
    serverlist->CallBackFn(serverlist, LIST_STATECHANGED, serverlist->instance, NULL, NULL);
}

/* Validate us to the master and send a list request. */
#define SECURE "\\secure\\"
static GError SendListRequest(GServerList serverlist)
{
    char data[256], *ptr, result[64];
    int len, total;
    unsigned long deadline;

    total = 0;
    data[0] = '\0';
    deadline = current_time() + MASTER_SECURE_TIMEOUT;
    for (;;)
    {
        len = NET_ReadFromStreamSocket(serverlist->slsocket, data + total, (int)sizeof(data) - total - 1);
        if (len < 0)
            return GE_NOCONNECT;
        if (len > 0)
        {
            total += len;
            data[total] = '\0';
            ptr = strstr(data, SECURE);
            if (ptr && strlen(ptr + strlen(SECURE)) >= 6)
                break;
        }
        if (current_time() >= deadline || total >= (int)sizeof(data) - 1)
            return GE_NOCONNECT;
        msleep(10);
    }

    ptr += strlen(SECURE);

    gs_encrypt((uchar *)serverlist->seckey, 6, ptr, 6);
    gs_encode((uchar *)ptr, 6, (uchar *)result);

    sprintf(data, "\\gamename\\%s\\gamever\\%s\\location\\0\\validate\\%s\\final\\\\queryid\\1.1\\",
        serverlist->enginename, ENGINE_VERSION, result);
    if (!NET_WriteToStreamSocket(serverlist->slsocket, data, (int)strlen(data)))
        return GE_NOCONNECT;

    sprintf(data, "\\list\\gamename\\%s\\final\\", serverlist->gamename);
    if (!NET_WriteToStreamSocket(serverlist->slsocket, data, (int)strlen(data)))
        return GE_NOCONNECT;

    ServerListModeChange(serverlist, sl_listxfer);
    return GE_NOERROR;
}

static GError SendBroadcastRequest(GServerList serverlist, int startport, int endport, int delta)
{
    NET_Address *broadcast;
    short i;
    static const char status[] = "\\status\\";

    broadcast = GServerResolveAddress("255.255.255.255");
    if (!broadcast)
        return GE_NODNS;

    for (i = (short)startport; i <= (short)endport; i += (short)delta)
    {
        NET_SendDatagram(serverlist->update_socket, broadcast, (Uint16)i, status, 8);
    }

    NET_UnrefAddress(broadcast);
    ServerListModeChange(serverlist, sl_lanlist);
    serverlist->lanstarttime = current_time();
    return GE_NOERROR;
}

static void DoSyncLoop(GServerList serverlist)
{
    while (serverlist->state != sl_idle)
    {
        ServerListThink(serverlist);
        msleep(10);
    }
}

GError ServerListUpdate(GServerList serverlist, gbool async)
{
    GError error;

    if (serverlist->state != sl_idle)
        return GE_BUSY;

    error = InitUpdateList(serverlist);
    if (error)
        return error;

    error = CreateServerListSocket(serverlist);
    if (error)
    {
        FreeUpdateList(serverlist);
        return error;
    }

    error = SendListRequest(serverlist);
    if (error)
    {
        GServerDestroyStream(serverlist);
        FreeUpdateList(serverlist);
        return error;
    }

    serverlist->nextupdate = 0;
    serverlist->abortupdate = 0;
    if (!async)
        DoSyncLoop(serverlist);
    return GE_NOERROR;
}

GError ServerListLANUpdate(GServerList serverlist, gbool async, int startsearchport, int endsearchport, int searchdelta)
{
    GError error;

    assert(searchdelta > 0);
    if (serverlist->state != sl_idle)
        return GE_BUSY;

    error = InitUpdateList(serverlist);
    if (error)
        return error;

    error = SendBroadcastRequest(serverlist, startsearchport, endsearchport, searchdelta);
    if (error)
    {
        FreeUpdateList(serverlist);
        return error;
    }

    serverlist->nextupdate = 0;
    serverlist->abortupdate = 0;
    if (!async)
        DoSyncLoop(serverlist);
    return GE_NOERROR;
}

static void ServerListAddServer(GServerList serverlist, char *ip, int port)
{
    GServer server;
    server = ServerNew(ip, port);
    ArrayAppend(serverlist->servers, &server);
}

static GError ServerListLANList(GServerList serverlist)
{
    char indata[1500];

    while (serverlist->update_socket)
    {
        NET_Datagram *datagram = NULL;
        int error;

        if (!NET_ReceiveDatagram(serverlist->update_socket, &datagram) || !datagram)
            break;
        error = datagram->buflen;

        if (error < (int)sizeof(indata))
        {
            memcpy(indata, datagram->buf, error);
            indata[error] = '\0';
            if (strstr(indata, "\\final\\") != NULL && datagram->addr)
            {
                const char *ip = NET_GetAddressString(datagram->addr);
                if (ip)
                    ServerListAddServer(serverlist, (char *)ip, datagram->port);
            }
        }
        NET_DestroyDatagram(datagram);
    }
    return GE_NOERROR;
}

static GError ServerListReadList(GServerList serverlist)
{
    static char data[2048];
    int len, oldlen;
    char *p, ip[32], port[16], *q, *lastip;

    oldlen = (int)strlen(data);
    len = NET_ReadFromStreamSocket(serverlist->slsocket, data + oldlen, sizeof(data) - oldlen - 1);
    if (len < 0)
        return GE_NOCONNECT;
    if (len == 0)
        return GE_NOERROR;

    data[len + oldlen] = 0;
    lastip = data;
    while (*lastip != '\0')
    {
        p = lastip;
        if (*(p + 1) == 'f' || serverlist->abortupdate)
        {
            GServerDestroyStream(serverlist);
            data[0] = 0;
            ServerListModeChange(serverlist, sl_querying);
            return GE_NOERROR;
        }
        if (strlen(p) < 14)
            break;
        p += 4;
        if (strchr(p, ':') == NULL || strchr(p, '\\') == NULL)
            break;

        q = ip;
        while (*p != 0 && *p != ':')
            *q++ = *p++;
        *q = '\0';
        p++;
        q = port;
        while (*p != 0 && *p != '\\')
            *q++ = *p++;
        *q = '\0';
        lastip = p;
        ServerListAddServer(serverlist, ip, atoi(port));
    }

    memmove(data, lastip, strlen(lastip) + 1);
    return GE_NOERROR;
}

#define STATUS "\\status\\"
static GError ServerListQueryLoop(GServerList serverlist)
{
    int i, scount = 0, error, final;
    char indata[1500];
    GServer server;

    if (serverlist->update_socket)
    {
        for (;;)
        {
            NET_Datagram *datagram = NULL;
            const char *ip;

            if (!NET_ReceiveDatagram(serverlist->update_socket, &datagram) || !datagram)
                break;
            error = datagram->buflen;
            if (error >= (int)sizeof(indata))
            {
                NET_DestroyDatagram(datagram);
                continue;
            }

            memcpy(indata, datagram->buf, error);
            indata[error] = 0;
            final = (strstr(indata, "\\final\\") != NULL);
            ip = datagram->addr ? NET_GetAddressString(datagram->addr) : NULL;

            if (ip)
            {
                for (i = 0; i < serverlist->maxupdates; i++)
                {
                    if (serverlist->updatelist[i].serverindex >= 0)
                    {
                        server = *(GServer *)ArrayNth(serverlist->servers, serverlist->updatelist[i].serverindex);
                        if (!strcmp(ServerGetAddress(server), ip) &&
                            ServerGetQueryPort(server) == datagram->port)
                        {
                            if (server->ping == 9999)
                                server->ping = current_time() - serverlist->updatelist[i].starttime;
                            ServerParseKeyVals(server, indata);
                            if (final)
                            {
                                serverlist->CallBackFn(serverlist,
                                    LIST_PROGRESS,
                                    serverlist->instance,
                                    server,
                                    (void *)((serverlist->nextupdate * 100) / ArrayLength(serverlist->servers)));
                                serverlist->updatelist[i].serverindex = -1;
                            }
                            break;
                        }
                    }
                }
            }

            NET_DestroyDatagram(datagram);
        }
    }

    for (i = 0; i < serverlist->maxupdates; i++)
    {
        if (serverlist->updatelist[i].serverindex >= 0)
        {
            scount++;
            if (current_time() - serverlist->updatelist[i].starttime > SERVER_TIMEOUT)
                serverlist->updatelist[i].serverindex = -1;
        }
    }

    scount = 0;
    for (i = 0; i < serverlist->maxupdates; i++)
        if (serverlist->updatelist[i].serverindex >= 0)
            scount++;

    if (serverlist->abortupdate ||
        (serverlist->nextupdate >= ArrayLength(serverlist->servers) && scount == 0))
    {
        FreeUpdateList(serverlist);
        ServerListModeChange(serverlist, sl_idle);
        return GE_NOERROR;
    }

    for (i = 0; i < serverlist->maxupdates && serverlist->nextupdate < ArrayLength(serverlist->servers); i++)
    {
        if (serverlist->updatelist[i].serverindex < 0)
        {
            NET_Address *address;
            int server_index;

            server_index = serverlist->nextupdate++;
            server = *(GServer *)ArrayNth(serverlist->servers, server_index);
            address = GServerResolveAddress(ServerGetAddress(server));
            if (!address)
                continue;

            if (NET_SendDatagram(serverlist->update_socket, address,
                                  (Uint16)ServerGetQueryPort(server),
                                  STATUS, (int)strlen(STATUS)))
            {
                serverlist->updatelist[i].serverindex = server_index;
                serverlist->updatelist[i].starttime = current_time();
            }
            NET_UnrefAddress(address);
        }
    }

    return GE_NOERROR;
}

/* ServerListThink
------------------
For use with Async Updates. This needs to be called every ~10ms for list processing and
updating to occur during async server list updates. */
GError ServerListThink(GServerList serverlist)
{
    switch (serverlist->state)
    {
        case sl_idle:
            return GE_NOERROR;
        case sl_listxfer:
            return ServerListReadList(serverlist);
        case sl_lanlist:
            return ServerListLANList(serverlist);
        case sl_querying:
            return ServerListQueryLoop(serverlist);
    }

    return GE_NOERROR;
}

/* ServerListHalt
-----------------
Halts the current update batch */
GError ServerListHalt(GServerList serverlist)
{
	if (serverlist->state != sl_idle)
		serverlist->abortupdate = 1;

	return 0;
}

/* ServerListClear
------------------
Clear and free all of the servers from the server list.
List must be in the sl_idle state */
GError ServerListClear(GServerList serverlist)
{
	
	if (serverlist->state != sl_idle)
		return GE_BUSY;
	//fastest way to clear is kill and recreate
	ArrayFree(serverlist->servers);
	serverlist->servers = ArrayNew(sizeof(GServer), SERVER_GROWBY, ServerFree);
	return 0;
}

/* ServerListState
------------------
Returns the current state of the server list */
GServerListState ServerListState(GServerList serverlist)
{
	return serverlist->state;
}

/* ServerListErrorDesc
----------------------
Returns a static string description of the specified error */
char *ServerListErrorDesc(GServerList serverlist, GError error)
{
	switch (error)
	{
	case GE_NOERROR: return "";
	case GE_NOSOCKET: return "Unable to create socket";
	case GE_NODNS: return "Unable to resolve master";
	case GE_NOCONNECT: return "Connection to master reset";
	case GE_BUSY: return "Server List is busy";
	}
	return "UNKNOWN ERROR CODE";

}

/* ServerListGetServer
----------------------
Returns the server at the specified index, or NULL if the index is out of bounds */
GServer ServerListGetServer(GServerList serverlist, int index)
{
	if (index < 0 || index >= ArrayLength(serverlist->servers))
		return NULL;
	return *(GServer *)ArrayNth(serverlist->servers,index);
}

/* ServerListCount
------------------
Returns the number of servers on the specified list. Indexing is 0 based, so
the actual server indexes are 0 <= valid index < Count */
int ServerListCount(GServerList serverlist)
{
	return ArrayLength(serverlist->servers);
}

/****
Comparision Functions
***/
static int IntKeyCompare(const void *entry1, const void *entry2)
{
	GServer server1 = *(GServer *)entry1, server2 = *(GServer *)entry2;
	int diff;
	diff = ServerGetIntValue(server1, g_sortserverlist->sortkey, 0) - 
			ServerGetIntValue(server2, g_sortserverlist->sortkey, 0);
	if (!g_sortserverlist->sortascending) 
		diff = -diff;
	return diff;
	
}

static int FloatKeyCompare(const void *entry1, const void *entry2)
{
    GServer server1 = *(GServer *)entry1, server2 = *(GServer *)entry2;
	double f = ServerGetFloatValue(server1, g_sortserverlist->sortkey, 0) - 
			ServerGetFloatValue(server2, g_sortserverlist->sortkey, 0);
	if (!g_sortserverlist->sortascending) 
		f = -f;
	if (f > 0) return 1; else if (f < 0) return -1; else return 0;
}

static int StrCaseKeyCompare(const void *entry1, const void *entry2)
{
	
    GServer server1 = *(GServer *)entry1, server2 = *(GServer *)entry2;
    int diff = strcmp(ServerGetStringValue(server1, g_sortserverlist->sortkey, ""),
				ServerGetStringValue(server2, g_sortserverlist->sortkey, ""));
	if (!g_sortserverlist->sortascending) 
		diff = -diff;
	return diff;
}

static int StrNoCaseKeyCompare(const void *entry1, const void *entry2)
{
	GServer server1 = *(GServer *)entry1, server2 = *(GServer *)entry2;
	int diff = strcasecmp(ServerGetStringValue(server1, g_sortserverlist->sortkey, ""),
				ServerGetStringValue(server2, g_sortserverlist->sortkey, ""));
	if (!g_sortserverlist->sortascending) 
		diff = -diff;
	return diff;
}

/* ServerListSort
-----------------
Sort the server list in either ascending or descending order using the 
specified comparemode.
sortkey can be a normal server key, or "ping" or "hostaddr" */
void ServerListSort(GServerList serverlist, gbool ascending, char *sortkey, GCompareMode comparemode)
{
	ArrayCompareFn comparator;
	switch (comparemode)
	{
	case cm_int: comparator = IntKeyCompare;
		break;
	case cm_float: comparator = FloatKeyCompare;
		break;
	case cm_strcase: comparator = StrCaseKeyCompare;
		break;
	case cm_stricase: comparator = StrNoCaseKeyCompare;
		break;
	}
	serverlist->sortkey = sortkey;
	serverlist->sortascending = ascending;
	g_sortserverlist = serverlist;
	ArraySort(serverlist->servers,comparator);

}


#ifdef __cplusplus
}
#endif

