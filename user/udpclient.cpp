#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <SDL3/SDL.h>
#include <SDL3_net/SDL_net.h>
#include "dk_shared.h"

int g_CommonSocket;
cvar_t *g_cvarLogServer;

static NET_DatagramSocket *g_udpSocket = NULL;
static NET_Address *g_logServerAddress = NULL;

static void UDP_ClearSocket(void)
{
   if(g_udpSocket)
      NET_DestroyDatagramSocket(g_udpSocket);
   g_udpSocket = NULL;

   if(g_logServerAddress)
      NET_UnrefAddress(g_logServerAddress);
   g_logServerAddress = NULL;

   g_CommonSocket = 0;
}

bool UDP_SenderSocket(void)
{
   NET_Address *address;
   NET_DatagramSocket *socket;

   if(!g_cvarLogServer || !g_cvarLogServer->string || !strlen(g_cvarLogServer->string))
      return false;

   UDP_ClearSocket();

   address = NET_ResolveHostname(g_cvarLogServer->string);
   if(!address)
      return false;

   if(NET_WaitUntilResolved(address, -1) < 0)
   {
      NET_UnrefAddress(address);
      return false;
   }

   if(!NET_GetAddressString(address))
   {
      NET_UnrefAddress(address);
      return false;
   }

   socket = NET_CreateDatagramSocket(NULL, 0, 0);
   if(!socket)
   {
      NET_UnrefAddress(address);
      return false;
   }

   g_udpSocket = socket;
   g_logServerAddress = address;
   g_CommonSocket = 1;
   return true;
}

int UDP_Send(int len, unsigned char *buf)
{
   if(!buf || !len)
      return 0;

   if(!g_CommonSocket && !UDP_SenderSocket())
      return 0;

   return NET_SendDatagram(g_udpSocket, g_logServerAddress, 2442, buf, len) ? len : 0;
}

int UDP_Log(int msgtype, char *fmt, ...)
{
   va_list argptr;
   unsigned char formatted[1024] = {"\0"};
   float t;
   time_t now;

   va_start(argptr, fmt);
   vsprintf((char *)&formatted[5], fmt, argptr);
   va_end(argptr);

   formatted[0]=(unsigned char)msgtype;
   now = time(NULL);
   t=(float)now;
   memcpy(&formatted[1], &t, 4);
   return UDP_Send((int)strlen((char *)&formatted[5]) + 5, (unsigned char *)formatted);
}

void UDP_ShutDown(void)
{
   UDP_ClearSocket();
}

long NetworkLong(long hostlong)
{
   return (long)SDL_Swap32BE((Uint32)hostlong);
}

long HostLong(long netlong)
{
   return (long)SDL_Swap32BE((Uint32)netlong);
}
