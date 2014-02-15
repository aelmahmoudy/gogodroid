/*
-----------------------------------------------------------------------------
 $Id: pal_socket.h,v 1.1 2009/11/20 16:38:53 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------

  Platform abstraction layer for socket.

-----------------------------------------------------------------------------
*/
#ifndef __PAL_SOCKET_H__
#define __PAL_SOCKET_H__


typedef sint32_t pal_socket_t;

// socket API definitions.
#include "pal_socket.def"
#include <sys/socket.h>


// socket shutdown modes.
#define PAL_SOCK_SHTDN_BOTH   SHUT_RDWR
#define PAL_SOCK_SHTDN_READ   SHUT_RD
#define PAL_SOCK_SHTDN_WRITE  SHUT_WR


// socket functions already available in this platform.
#undef pal_socket
#define pal_socket (pal_socket_t)socket

#undef pal_connect
#define pal_connect connect

#undef pal_shutdown
#define pal_shutdown shutdown

#undef pal_closesocket
#define pal_closesocket close

#endif
