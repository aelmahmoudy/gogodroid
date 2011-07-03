/*
-----------------------------------------------------------------------------
 $Id: net_echo_request.h,v 1.1 2009/11/20 16:53:15 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

#ifndef _NET_ECHO_REQUEST_H_
#define _NET_ECHO_REQUEST_H_

#define ECHO_REQUEST_COMMAND        "ECHO REQUEST"
#define ECHO_REQUEST_SUCCESS_STATUS 200
#define ECHO_REQUEST_IN_BUF_SIZE    4096
#define ECHO_REQUEST_OUT_BUF_SIZE   4096

#define ECHO_REQUEST_TIMEOUT        10 * 1000
#define ECHO_REQUEST_TIMEOUT_ADJUST ECHO_REQUEST_TIMEOUT * 2
#define ECHO_REQUEST_ERROR_ADJUST   ECHO_REQUEST_TIMEOUT * 2
#define ECHO_REQUEST_WRONG_FAMILY_ADJUST  ECHO_REQUEST_TIMEOUT * 3

#define ECHO_REQUEST_ATTEMPTS       3

typedef enum {
  SOCKET_ADDRESS_OK,
  SOCKET_ADDRESS_WRONG_FAMILY,
  SOCKET_ADDRESS_PROBLEM_RESOLVING,
  SOCKET_ADDRESS_ERROR
} tSocketAddressStatus;

extern tRedirectStatus tspDoEchoRequest(char *address, tBrokerAddressType address_type, tConf *conf, uint32_t *distance);

#endif
