/*
-----------------------------------------------------------------------------
 $Id: tsp_client.h,v 1.3 2010/03/07 19:53:19 carl Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

#ifndef _TSP_CLIENT_H_
#define _TSP_CLIENT_H_

#include "config.h"   /* tConf */
#include "xml_tun.h"  /* tTunnel , tspXMLParse() */
#include "tsp_net.h"  /* tPayload */
#include "net.h"      /* net_tools_t */
#include "tsp_redirect.h"

typedef enum
{
  CLIENT_VERSION_INDEX_2_0_2,
  CLIENT_VERSION_INDEX_2_0_1,
  CLIENT_VERSION_INDEX_2_0_0,
  CLIENT_VERSION_INDEX_1_0_1
} tClientVersionIndex;

#define CLIENT_VERSION_INDEX_CURRENT  CLIENT_VERSION_INDEX_2_0_2
#ifndef CLIENT_VERSION_INDEX_OLDEST	/* Allow platform override */
#define CLIENT_VERSION_INDEX_OLDEST   CLIENT_VERSION_INDEX_1_0_1
#endif

#define CLIENT_VERSION_INDEX_V6UDPV4_START  CLIENT_VERSION_INDEX_2_0_0
#define CLIENT_VERSION_INDEX_V4V6_START   CLIENT_VERSION_INDEX_2_0_0

#define CLIENT_VERSION_STRING_2_0_2   "2.0.2"
#define CLIENT_VERSION_STRING_2_0_1   "2.0.1"
#define CLIENT_VERSION_STRING_2_0_0   "2.0.0"
#define CLIENT_VERSION_STRING_1_0_1   "1.0.1"

// Defined locally in tsp_client.c
extern const char * TSPProtoVerStr[];

// Implemented in tsp_client.c
sint32_t            tspMain               ( sint32_t, char *[] );
sint32_t            tspExtractPayload     ( char *, tTunnel * );
sint32_t            tspGetStatusCode      ( char * );
const char*         tspGetTspStatusStr    ( sint32_t );

// Implemented in each platform tsp_local.c
extern uint16_t     tspGetLocalPort       ( pal_socket_t );
extern gogoc_status  tspStartLocal         ( pal_socket_t, tConf *, 
                                            tTunnel *, net_tools_t * );
extern void         tspSetEnv             ( char *, char *, sint32_t );
extern sint32_t     tspSetupInterfaceLocal( tConf* pConf, tTunnel* pTun );
extern sint32_t     tspCheckForStopOrWait ( const uint32_t uiWaitMs );
extern void         tspGetOSInfo          ( const size_t len, char* buf );
extern char*        tspGetLocalAddress    ( pal_socket_t, char *, sint32_t );
#ifdef DSLITE_SUPPORT
extern char*        tspGetRemoteAddress   ( pal_socket_t, char *, sint32_t );
#endif

#ifdef ANDROID
void writepid();
#endif

#endif

