/*
-----------------------------------------------------------------------------
 $Id: net_tcp6.c,v 1.1 2009/11/20 16:53:40 jasminko Exp $
-----------------------------------------------------------------------------
* This source code copyright (c) gogo6 Inc. 2002-2004,2007.
* 
* This program is free software; you can redistribute it and/or modify it 
* under the terms of the GNU General Public License (GPL) Version 2, 
* June 1991 as published by the Free Software Foundation.
* 
* This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY;  without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
* See the GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License 
* along with this program; see the file GPL_LICENSE.txt. If not, write 
* to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
* MA 02111-1307 USA
-----------------------------------------------------------------------------
*/

#include "platform.h"

#include "net_tcp6.h"
#include "net.h" /* NetText2Addr */


// --------------------------------------------------------------------------
// NetTCP6Connect:
//
// Return values:
//    0: success
//   -1: Failed to resolve srvname.
//   -2: Socket error / Failed to connect (TCP only).
//
sint32_t NetTCP6Connect( pal_socket_t *p_sock, char *Host, uint16_t Port ) 
{
  pal_socket_t         sockfd;
  struct sockaddr_in6  serv_addr;
  struct in6_addr      addr;

  if( NULL == NetText2Addr6(Host, &addr) )
  {
    return -1;
  }

  memset(&serv_addr, 0, sizeof(serv_addr)); 
  serv_addr.sin6_family      = AF_INET6;
  serv_addr.sin6_port        = htons(Port);
  serv_addr.sin6_addr = addr;

  /*
   * Open a TCP socket (an Internet 6 stream socket).
   */
  if( (sockfd = pal_socket(AF_INET6, SOCK_STREAM, 0)) < 0 )
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


/* */
sint32_t NetTCP6Close( pal_socket_t Socket )
{
  pal_shutdown( Socket, PAL_SOCK_SHTDN_BOTH );
  return pal_closesocket( Socket );
}


/* */
sint32_t NetTCP6ReadWrite( pal_socket_t sock, char *bi, sint32_t li, char *bo, sint32_t lo )
{
  if ( NetTCP6Write(sock, bi, li) != li)
    return -1;

  return NetTCP6Read(sock, bo, lo);
}


/* */
sint32_t NetTCP6Write( pal_socket_t sock, char *b, sint32_t l )
{
  sint32_t nleft, nwritten;
  char *ptr;

  ptr = b;   /* can't do pointer arithmetic on void * */
  nleft = l;
  while( nleft > 0 )
  {
    if( (nwritten = send(sock, ptr, nleft, 0)) <= 0 )
    {
      return nwritten;          /* error */
    }

    nleft -= nwritten;
    ptr   += nwritten;
  }
  return(l);
}


/* */
sint32_t NetTCP6Printf( pal_socket_t sock, char *out, sint32_t pl, char *Format, ... )
{
  va_list argp;
  sint32_t Length;
  char Data[1024];

  va_start(argp, Format);
  pal_vsnprintf(Data, sizeof Data, Format, argp);
  va_end(argp);

  Length = pal_strlen(Data);

  if( NetTCP6Write(sock, Data, pal_strlen(Data)) != Length )
  {
    return 0;
  }

  return NetTCP6Read(sock, out, pl);
}


/* */ 
sint32_t NetTCP6Read( pal_socket_t sock, char *in, sint32_t l )
{
  return( recv(sock, in, l, 0) );
}
