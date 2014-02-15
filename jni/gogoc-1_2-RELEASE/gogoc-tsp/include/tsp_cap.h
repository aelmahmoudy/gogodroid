/*
-----------------------------------------------------------------------------
 $Id: tsp_cap.h,v 1.1 2009/11/20 16:53:17 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

#ifndef _TSP_CAP_H_
#define _TSP_CAP_H_

#include "net.h"
#include "tsp_redirect.h"

/*
   Capability bytes

   16      8      1
   |-------|------|
               `--' - TUNNEL TYPE 1-4
           `--' AUTH METHOD 5-8
   `------' RESERVED 9-16
*/

/* the tunnel modes values correspond to tTunnelMode defined in config.h */
#define TUNNEL_V6V4         0x0001
#define TUNNEL_V6UDPV4      0x0002
#define TUNNEL_ANY          0x0003
#define TUNNEL_V4V6         0x0004

/* Authentication values */
#ifndef NO_OPENSSL
#define AUTH_PASSDSS_3DES_1 0x0080
#endif
#define AUTH_DIGEST_MD5     0x0040
#define AUTH_PLAIN          0x0020
#define AUTH_ANONYMOUS      0x0010
#define AUTH_ANY            0x00F0

typedef uint32_t tCapability;

tCapability         tspSetCapability      ( char *, char * );
gogoc_status         tspGetCapabilities    ( pal_socket_t, net_tools_t *, tCapability *, int, tConf *, tBrokerList ** );
char*               tspFormatCapabilities ( char* szBuffer, const size_t bufLen, const tCapability cap );

#endif
