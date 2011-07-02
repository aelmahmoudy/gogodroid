/*
-----------------------------------------------------------------------------
 $Id: net_udp.h,v 1.1 2009/11/20 16:53:16 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#ifndef _NET_UDP_H_
#define _NET_UDP_H_

extern sint32_t     NetUDPConnect         (pal_socket_t *, char *, uint16_t );
extern sint32_t     NetUDPClose           (pal_socket_t);

extern sint32_t     NetUDPReadWrite       (pal_socket_t, char *, sint32_t, char *, sint32_t);

extern sint32_t     NetUDPWrite           (pal_socket_t, char *, sint32_t);
extern sint32_t     NetUDPPrintf          (pal_socket_t, char *, sint32_t, char *, ...);

extern sint32_t     NetUDPRead            (pal_socket_t, char *, sint32_t);

#endif
