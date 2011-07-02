/*
---------------------------------------------------------------------------
 $Id: tsp_tun.h,v 1.1 2009/11/20 16:53:20 jasminko Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2006 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
  
---------------------------------------------------------------------------
*/

#ifndef TUN_H
#define TUN_H

#include "config.h"   // tBoolean

void                TunName               ( int tunfd, char* name, size_t name_len );
int                 TunInit               (char* name);
gogoc_status         TunMainLoop           (int tun, int Socket, tBoolean keepalive,
                                           int keepalive_interval, char *local_address_ipv6,
                                           char *keepalive_address);

#endif /* TUN_H */
