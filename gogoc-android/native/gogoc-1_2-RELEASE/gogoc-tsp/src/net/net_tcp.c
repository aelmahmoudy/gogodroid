/*
-----------------------------------------------------------------------------
 $Id: net_tcp.c,v 1.1 2009/11/20 16:53:40 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"

#include "net_tcp.h"
#include "net.h" /* NetText2Addr */


// --------------------------------------------------------------------------
// NetTCPConnect:
//
// Return values:
//    0: success
//   -1: Failed to resolve srvname.
//   -2: Socket error / Failed to connect (TCP only).
//
sint32_t NetTCPConnect( pal_socket_t *p_sock, char *Host, uint16_t Port ) 
{
  pal_socket_t        sockfd;
  struct sockaddr_in  serv_addr;
  struct in_addr      addr;

  if( NetText2Addr(Host, &addr) == NULL )
  {
    return -1;
  }

  memset(&serv_addr, 0, sizeof(serv_addr)); 
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_port        = htons(Port);
  serv_addr.sin_addr.s_addr = addr.s_addr;

  /*
   * Open a TCP socket (an Internet stream socket).
   */
  if( (sockfd = pal_socket(AF_INET, SOCK_STREAM, 0)) < 0 )
  {
    return -2;
  }

  /*
   * Connect to the server.
   */
  if( pal_connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 )
  {
    pal_closesocket( sockfd );
    return -2;
  }

  *p_sock = sockfd;
  return 0;
}


// --------------------------------------------------------------------------
sint32_t NetTCPClose( pal_socket_t Socket )
{
  pal_shutdown( Socket, PAL_SOCK_SHTDN_BOTH );
  return pal_closesocket(Socket);
}


/* */
sint32_t NetTCPReadWrite( pal_socket_t sock, char *bi, sint32_t li, char *bo, sint32_t lo )
{
  if( NetTCPWrite(sock, bi, li) != li )
    return -1;

  return NetTCPRead(sock, bo, lo);
}


/* */
sint32_t NetTCPWrite( pal_socket_t sock, char *b, sint32_t l ) 
{
  sint32_t nleft, nwritten;
  char *ptr;

  ptr = b;   /* can't do pointer arithmetic on void * */
  nleft = l;
  while (nleft > 0) 
  {
    if( (nwritten = send(sock, ptr, nleft, 0)) <= 0 )
    {
      return nwritten;          /* error */
    }

    nleft -= nwritten;
    ptr   += nwritten;
  }

  return l ;
}


/* */
sint32_t NetTCPPrintf( pal_socket_t sock, char *out, sint32_t pl, char *Format, ... )
{
  va_list argp;
  sint32_t Length;
  char Data[1024];

  va_start(argp, Format);
  pal_vsnprintf(Data, sizeof Data, Format, argp);
  va_end(argp);

  Length = pal_strlen(Data);

  if( NetTCPWrite(sock, Data, pal_strlen(Data)) != Length )
  {
    return 0;
  }

  return NetTCPRead(sock, out, pl);
}


/* */ 
sint32_t NetTCPRead( pal_socket_t sock, char *in, sint32_t l )
{
  return( recv(sock, in, l, 0) );
}





