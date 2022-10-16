#include <stdio.h>
//#include <windows.h>
#include "dk_system.h"
#include <winsock.h>
#include <sys/timeb.h>
#include "dk_shared.h"

int g_CommonSocket;
cvar_t *g_cvarLogServer;

///////////////////////////////////////////////////////////////////////
// UDP_SenderSocket()
//
// Description:
//    Acquires a udp socket, binds a socket address name, then puts it 
//    in a connected state. A connected UDP socket doesn't have to 
//    bother with name space init for each packet --efficient delivery,
//    though windows tends to drop packets around 100pps (see UDP_Send())
//
// Parms:
//
// Return:
//    Non-zero on success, otherwise false.
//
// Bugs:
//    Uses an unreliable protocol for delivery. No flow control.
//
bool UDP_SenderSocket(void)
{
   WORD                 version_requested;
   WSADATA              wsa_data;
   int                  err;
   unsigned int         i;
   bool                 alphaflag;
   struct sockaddr_in   sock_name;
   struct hostent       *host_entry;
   unsigned long        packed_net_addr;
   unsigned short       port = 2442;

   if(!g_cvarLogServer)
      return false;

   if(!strlen(g_cvarLogServer->string))
      return false;

   alphaflag = false;
   // Close old socket if there is a new connect request
   if(g_CommonSocket)
      closesocket(g_CommonSocket);

   g_CommonSocket = socket(AF_INET, SOCK_DGRAM, 0);

   if( (err = WSAGetLastError()) == WSANOTINITIALISED)
   {
      version_requested = MAKEWORD(2,0);
      err = WSAStartup(version_requested, &wsa_data);
      if(err)
      {
         return false;
      } else {
         g_CommonSocket = socket(AF_INET, SOCK_DGRAM, 0);
         if(err = WSAGetLastError())
         {
            return false;
         }
      }
   }


   sock_name.sin_family = AF_INET;
   sock_name.sin_port = htons(port);
   
   for(i=0; i < strlen(g_cvarLogServer->string); i++)
   {
      if(isalpha(g_cvarLogServer->string[i]))
      {
         alphaflag = true;
         break;
      }
   }

   if(alphaflag)
   {
      if( (host_entry = gethostbyname(g_cvarLogServer->string)) != NULL)
      {
         memcpy(&sock_name.sin_addr.s_addr, host_entry->h_addr, host_entry->h_length);
      } else {
         closesocket(g_CommonSocket);
         return false;
      }
   } else {
      if( (packed_net_addr = inet_addr(g_cvarLogServer->string)) != -1)
      {
         memcpy(&sock_name.sin_addr.s_addr, &packed_net_addr, sizeof(unsigned long));
      } else {
         closesocket(g_CommonSocket);
         return false;
      }
   }

   if(connect(g_CommonSocket, (struct sockaddr *)&sock_name, sizeof(struct sockaddr_in)))
   {
      closesocket(g_CommonSocket);
      return false;
   }

   return true;
}
// End UDP_SenderSocket()
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// UDP_Send(int len, unsigned char *buf)
//
// Description:
//    Sends a datagram on a "connected" UDP socket. UDP is a
//    fast, simple and efficient protocol, but be warned that
//    it is also unreliable and packets tend to drop on some
//    platforms (e.g. Win32) near 100pps. That is, try to 
//    make fewer than 100 calls / sec to UDP_Send() if you 
//    want to be reasonably sure that most/all of your data
//    is arriving in tact.
//
// Parms:
//    SOCKET *sock      pointer to a socket descriptor, use
//                      UDP_SenderSocket() to acquire one.
//    len               amount of data to be sent
//    buf               buffer containing data to be sent
//
int UDP_Send(int len, unsigned char *buf)
{
   if(! buf)
      return 0;
   if(! len)
      return 0;
   if(! g_CommonSocket)
      if(!UDP_SenderSocket())
         return 0;
   return send(g_CommonSocket, (char *)buf, len, 0);
}
// End UDP_Send()
/////////////////////////////////////////////////////////////

int UDP_Log(int msgtype, char *fmt, ...)
{
   struct _timeb  tb;
   va_list        argptr;
   unsigned char  formatted[1024] = {"\0"};
   float          t;

   va_start(argptr, fmt);
   vsprintf((char *)&formatted[5], fmt, argptr);
   va_end(argptr);

   formatted[0]=(unsigned char)msgtype;
   _ftime(&tb);
   t=(float)(tb.time + (tb.millitm / 1000));
   memcpy(&formatted[1], &t, 4);
   return UDP_Send((strlen((char *)&formatted[5]) + 5), (unsigned char *)formatted);
}

void UDP_ShutDown(void)
{
   if(g_CommonSocket)
      closesocket(g_CommonSocket);
   g_CommonSocket = 0;
}

/////////////////////////////////////////////////////////////
//

long NetworkLong(long hostlong)
{
   return htonl(hostlong);
}

long HostLong(long netlong)
{
   return ntohl(netlong);
}