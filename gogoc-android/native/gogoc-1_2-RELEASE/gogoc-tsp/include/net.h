/*
-----------------------------------------------------------------------------
 $Id: net.h,v 1.1 2009/11/20 16:53:15 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

#ifndef _NET_H_
#define _NET_H_

struct net_tools
{
  sint32_t      (*netopen)      (pal_socket_t *, char *, uint16_t);
  sint32_t      (*netclose)     (pal_socket_t);

  sint32_t      (*netsendrecv)  (pal_socket_t, char *, sint32_t, char *, sint32_t);

  sint32_t      (*netsend)      (pal_socket_t, char *, sint32_t);
  sint32_t      (*netprintf)    (pal_socket_t, char *, sint32_t, char *, ...);

  sint32_t      (*netrecv)      (pal_socket_t, char *, sint32_t);
  sint32_t      (*netreadline)  (char *, sint32_t, char*, sint32_t);
};

typedef struct net_tools net_tools_t;

#define NET_TOOLS_T_SIZE    5

#define NET_TOOLS_T_RUDP    0
#define NET_TOOLS_T_UDP     1
#define NET_TOOLS_T_TCP     2
#define NET_TOOLS_T_TCP6    3
#define NET_TOOLS_T_RUDP6   4

sint32_t            parse_addr_port       ( const char*, char**, uint16_t*, uint16_t );
struct in_addr *    NetText2Addr          ( char*, struct in_addr * );
struct in6_addr *   NetText2Addr6         ( char*, struct in6_addr * );

#endif
