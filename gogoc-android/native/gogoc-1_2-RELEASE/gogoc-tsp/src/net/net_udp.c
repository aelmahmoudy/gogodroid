/*
-----------------------------------------------------------------------------
 $Id: net_udp.c,v 1.1 2009/11/20 16:53:40 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"

#include "net_udp.h"
#include "net.h"


// --------------------------------------------------------------------------
// NetUDPConnect:
//
// Return values:
//    0: success
//   -1: Failed to resolve srvname.
//   -2: Socket error / Failed to connect (TCP only).
//
sint32_t NetUDPConnect(pal_socket_t *p_sock, char *Host, uint16_t Port)
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
   * Open a UDP socket.
   */
  if( (sockfd = pal_socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
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
sint32_t NetUDPClose(pal_socket_t Socket)
{
  pal_shutdown( Socket, PAL_SOCK_SHTDN_BOTH );
  return pal_closesocket(Socket);
}


/* */
sint32_t NetUDPReadWrite(pal_socket_t sock, char *bi, sint32_t li, char *bo, sint32_t lo) 
{
	if ( NetUDPWrite(sock, bi, li) != li)
		return -1;
	
	return NetUDPRead(sock, bo, lo);
}
	

/* */
sint32_t NetUDPWrite(pal_socket_t sock, char *b, sint32_t l) 
{
	sint32_t nwritten, nleft;
	char *ptr;

	ptr = b;	/* can't do pointer arithmetic on void* */
	nleft = l;
	while (nleft > 0) {
    nwritten = send(sock, ptr, nleft, 0);
		if (nwritten <= 0) {
			return nwritten;		/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return l;
}


/* */
sint32_t NetUDPPrintf(pal_socket_t sock, char *out, sint32_t ol, char *Format, ...) 
{
	va_list argp;
	char Data[1024];

	va_start(argp, Format);
	pal_vsnprintf(Data, sizeof Data, Format, argp);
	va_end(argp);

	return NetUDPReadWrite(sock, Data, pal_strlen(Data), out, ol);
}


/* */
sint32_t NetUDPRead( pal_socket_t sock, char *b, sint32_t l ) 
{
	return(recvfrom(sock, b, l, 0, NULL, NULL));
}
