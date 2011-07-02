/*
---------------------------------------------------------------------------
 $Id: tsp_tun.h,v 1.1 2010/03/07 19:39:09 carl Exp $
---------------------------------------------------------------------------
This source code copyright (c) gogo6 Inc. 2002-2005,2007.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with gogo6 Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef TUN_H
#define TUN_H

#include "config.h"
#include "net.h"            /* net_tools_t */

sint32_t            TunInit               (char *TunDevice);
gogoc_status         TunMainLoop           (sint32_t tunfd, pal_socket_t Socket, sint32_t redirect,
                                           tBoolean keepalive, sint32_t keepalive_interval,
                                           char *local_address_ipv6, char *keepalive_address);
gogoc_status         TunRedirect           (sint32_t tunfd, pal_socket_t Socket, net_tools_t *nt, sint32_t *redirect);

#endif /* TUN_H */
