/*
-----------------------------------------------------------------------------
 $Id: net_rudp6.h,v 1.1 2009/11/20 16:53:16 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

#ifdef V4V6_SUPPORT

#ifndef _rudp6_h_
#define _rudp6_h_

extern sint32_t     NetRUDP6Connect       (pal_socket_t *, char *, uint16_t);
extern sint32_t     NetRUDP6Close         (pal_socket_t);

extern sint32_t     NetRUDP6ReadWrite     (pal_socket_t, char *, sint32_t, char *, sint32_t);

extern sint32_t     NetRUDP6Write         (pal_socket_t, char *, sint32_t);
extern sint32_t     NetRUDP6Printf        (pal_socket_t, char *, sint32_t, char *, ...);

extern sint32_t     NetRUDP6Read          (pal_socket_t, char *, sint32_t);

#endif

#endif /* V4V6_SUPPORT */
