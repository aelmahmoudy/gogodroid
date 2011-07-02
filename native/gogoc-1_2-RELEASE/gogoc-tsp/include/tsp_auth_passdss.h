/*
-----------------------------------------------------------------------------
 $Id: tsp_auth_passdss.h,v 1.1 2009/11/20 16:53:16 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#ifndef _TSP_AUTH_PASSDSS_H_
#define _TSP_AUTH_PASSDSS_H_

#ifndef NO_OPENSSL

#include "net.h"
#include "tsp_redirect.h"

gogoc_status         AuthPASSDSS_3DES_1    (pal_socket_t s, net_tools_t *nt, 
                                           tConf *conf, tBrokerList **broker_list);

#endif

#endif
