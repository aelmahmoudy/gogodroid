/*
-----------------------------------------------------------------------------
 $Id: tsp_redirect.h,v 1.1 2009/11/20 16:53:17 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#ifndef _TSP_REDIRECT_H_
#define _TSP_REDIRECT_H_

#include <limits.h>
#include "config.h"
#include "xml_tun.h"

#define DEFAULT_REDIRECT_LAST_SERVER_FILE   "tsp-last-server.txt"
#define DEFAULT_REDIRECT_BROKER_LIST_FILE   "tsp-broker-list.txt"

#define MAX_REDIRECT_ADDRESS_LENGTH       255
#define MAX_REDIRECT_LAST_SERVER_LENGTH     255
#define MAX_REDIRECT_BROKER_LIST_LENGTH     255
#define MAX_REDIRECT_LAST_SERVER_LINE_LENGTH  255
#define MAX_REDIRECT_BROKER_LIST_LINE_LENGTH  255

#define MAX_REDIRECT_BROKERS_IN_LIST      50

#define MAX_BROKER_LIST_STRING_LENGTH     4096

#define MAX_REDIRECT_STATUS_LENGTH        1024

#define REDIRECT_RECEIVE_BUFFER_SIZE      4096

#define BROKER_LIST_STRING_START        "[ "
#define BROKER_LIST_STRING_END          " ]"
#define BROKER_LIST_STRING_SEPARATOR      ", "
#define BROKER_LIST_STRING_SUSPENSION     "..."

#define REDIRECT_STATUS_CODE_BASE       1000

typedef enum {
  TSP_REDIRECT_OK,
  TSP_REDIRECT_INTERNAL_ERR,
  TSP_REDIRECT_CANT_OPEN_FILE,
  TSP_REDIRECT_NO_LAST_SERVER,
  TSP_REDIRECT_CANT_WRITE_TO_FILE,
  TSP_REDIRECT_CANT_ADD_BROKER_TO_LIST,
  TSP_REDIRECT_CANT_EXTRACT_PAYLOAD,
  TSP_REDIRECT_CANT_CREATE_LIST,
  TSP_REDIRECT_CANT_SORT_LIST,
  TSP_REDIRECT_CANT_LOG_REDIRECTION,
  TSP_REDIRECT_CANT_ALLOCATE_MEM,
  TSP_REDIRECT_ADDRESS_TRUNCATED,
  TSP_REDIRECT_CANT_GET_DISTANCES,
  TSP_REDIRECT_CANT_INSERT_BROKER_IN_LIST,
  TSP_REDIRECT_CANT_SAVE_BROKER_LIST,
  TSP_REDIRECT_EMPTY_BROKER_LIST,
  TSP_REDIRECT_TOO_MANY_BROKERS,
  TSP_REDIRECT_CANT_INIT_THREAD_ARG,
  TSP_REDIRECT_CANT_CREATE_THREAD,
  TSP_REDIRECT_CANT_WAIT_FOR_THREAD,
  TSP_REDIRECT_ECHO_REQUEST_TIMEOUT,
  TSP_REDIRECT_ECHO_REQUEST_ERROR,
  TSP_REDIRECT_CANT_MALLOC_THREAD_ARRAY,
  TSP_REDIRECT_CANT_MALLOC_THREAD_ARGS
} tRedirectStatus;

typedef enum {
  TSP_REDIRECT_BROKER_TYPE_NONE,
  TSP_REDIRECT_BROKER_TYPE_FQDN,
  TSP_REDIRECT_BROKER_TYPE_IPV4,
  TSP_REDIRECT_BROKER_TYPE_IPV6
} tBrokerAddressType;

typedef struct stBrokerList {
  char address[MAX_REDIRECT_ADDRESS_LENGTH];
  tBrokerAddressType address_type;
  unsigned int distance;
  struct stBrokerList *next;
} tBrokerList;

typedef struct stBrokerTimingThreadArg {
  tBrokerList *broker;
  tConf *conf;
} tBrokerTimingThreadArg;

extern tRedirectStatus tspGetBrokerDistances(tBrokerList *broker_list, int broker_count, tConf *conf);

extern int tspIsRedirectStatus(int status);
extern tRedirectStatus tspLogRedirectionList(tBrokerList *broker_list, int sorted);
extern tRedirectStatus tspFreeBrokerList(tBrokerList *broker_list);
extern tRedirectStatus tspHandleRedirect(char *payload, tConf *conf, tBrokerList **broker_list);
extern tRedirectStatus tspReadLastServerFromFile(char *last_server_file, char *buffer);
extern tRedirectStatus tspWriteLastServerToFile(char *last_server_file, char *last_server);
extern tRedirectStatus tspReadBrokerListFromFile(char *broker_list_file, tBrokerList **broker_list);
extern tRedirectStatus tspWriteBrokerListToFile(char *broker_list_file, tBrokerList *broker_list);

#endif

