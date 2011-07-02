/*
-----------------------------------------------------------------------------
 $Id: tsp_auth.h,v 1.1 2009/11/20 16:53:16 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

#ifndef _TSP_AUTH_H_
#define _TSP_AUTH_H_

#include "tsp_cap.h"
#include "net.h"
#include "tsp_redirect.h"

/**Bug1455:
 * If connecting to a Migration Server using TSP version 2.0.0 or earlier,
 * MD5 digest authentication may be wrongly calculated in some
 * username and other credentials combinations. The define below ensure
 * an MD5 digest compatible with earlier Migration Broker is generated
 * when the TSP protocol version is 2.0.0 or earlier.
 */
#define SUPPORT_MD5_BUG1455 1

gogoc_status         tspAuthenticate       (pal_socket_t, tCapability, 
                                           net_tools_t *, tConf *, 
                                           tBrokerList **, sint32_t);

#endif
