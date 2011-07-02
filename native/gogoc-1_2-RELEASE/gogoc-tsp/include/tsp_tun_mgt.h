/*
---------------------------------------------------------------------------
 $Id: tsp_tun_mgt.h,v 1.1 2009/11/20 16:53:17 jasminko Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
---------------------------------------------------------------------------
*/

#ifndef __TSP_TUN_MGT_H__
#define __TSP_TUN_MGT_H__


typedef struct __TUNNEL_LOOP_CONFIG
{
  char*         ka_src_addr;    // KA source address (usually local endpoint).
  char*         ka_dst_addr;    // KA destination address (usually broker).
  int           sa_family;      // Socket address family [AF_INET, AF_INET6].
  unsigned int  ka_interval;    // Keepalive interval in seconds.
  long          tun_lifetime;   // Tunnel lifetime (tunnel expiration feature).
} TUNNEL_LOOP_CONFIG, *PTUNNEL_LOOP_CONFIG;


gogoc_status         tspPerformTunnelLoop  ( const PTUNNEL_LOOP_CONFIG pTunLoopCfg );


#endif
