/*
-----------------------------------------------------------------------------
 $Id: tsp_client.c,v 1.6 2010/03/08 23:41:05 carl Exp $
-----------------------------------------------------------------------------
Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include <assert.h>

#include "platform.h"
#include "gogoc_status.h"

#include "tsp_client.h"

#include "net_udp.h"
#include "net_rudp.h"
#include "net_rudp6.h"
#include "net_tcp.h"
#include "net_tcp6.h"

#include "net.h"
#include "config.h"
#include "tsp_cap.h"
#include "tsp_auth.h"
#include "tsp_net.h"
#include "xml_tun.h"
#include "xml_req.h"
#include "tsp_redirect.h"

#include "version.h"
#include "log.h"
#include "hex_strings.h"


// The gogoCLIENT Messaging subsystem.
#include <gogocmessaging/clientmsgdataretriever.h>
#include <gogocmessaging/clientmsgnotifier.h>
#include <gogocmessaging/gogoc_c_wrapper.h>

#ifdef HACCESS
#include "haccess.h"
#endif

#define CONSEC_RETRY_TO_DOUBLE_WAIT   3   // Consecutive failed connection retries before doubling wait time.
#define TSP_VERSION_FALLBACK_DELAY    5

// --------------------------------------------------------------------------
const char *TSPProtoVerStr[] = {
  CLIENT_VERSION_STRING_2_0_2,
  CLIENT_VERSION_STRING_2_0_1,
  CLIENT_VERSION_STRING_2_0_0,
  CLIENT_VERSION_STRING_1_0_1,
  NULL
};

const char *GOGOCStatusContext[] = {
  STR_CTX_UNSPECIFIED,
  STR_CTX_CFGVALIDATION,
  STR_CTX_NETWORKINIT,
  STR_CTX_NETWORKCONNECT,
  STR_CTX_TSPCAPABILITIES,
  STR_CTX_TSPAUTHENTICATION,
  STR_CTX_TSPTUNNEGOTIATION,
  STR_CTX_TUNINTERFACESETUP,
  STR_CTX_TUNNELLOOP,
  STR_CTX_GOGOCTEARDOWN,
  NULL
};

struct __TSP_PROTOCOL_CODES_STR
{
  sint32_t status;
  const char* status_str;
} 
TSP_PROTOCOL_CODES_STR[] = {
  { TSP_PROTOCOL_SUCCESS,                    STR_TSP_PROTO_SUCCESS },
  { TSP_PROTOCOL_AUTH_FAILED,                STR_TSP_PROTO_AUTH_FAILED },
  { TSP_PROTOCOL_NO_TUNNELS,                 STR_TSP_PROTO_NO_TUNNELS },
  { TSP_PROTOCOL_UNSUP_TSP_VER,              STR_TSP_PROTO_UNSUP_TSP_VER },
  { TSP_PROTOCOL_UNSUP_TUN_MODE,             STR_TSP_PROTO_UNSUP_TUN_MODE },
  { TSP_PROTOCOL_UNDEFINED,                  STR_TSP_PROTO_UNDEFINED },
  { TSP_PROTOCOL_INVALID_REQUEST,            STR_TSP_PROTO_INVALID_REQUEST },
  { TSP_PROTOCOL_INVALID_IPV4,               STR_TSP_PROTO_INVALID_IPV4 },
  { TSP_PROTOCOL_INVALID_IPV6,               STR_TSP_PROTO_INVALID_IPV6 },
  { TSP_PROTOCOL_IPV4_PREFIX_ALREADY_USED,   STR_TSP_PROTO_IPV4_PREFIX_ALREADY_USED },
  { TSP_PROTOCOL_REQ_PREFIX_LEN_UNAVAILABLE, STR_TSP_PROTO_REQ_PREFIX_LEN_UNAVAILABLE },
  { TSP_PROTOCOL_DNS_DELEGATION_ERROR,       STR_TSP_PROTO_DNS_DELEGATION_ERROR },
  { TSP_PROTOCOL_UNSUPP_PREFIX_LEN,          STR_TSP_PROTO_UNSUPP_PREFIX_LEN },
  { TSP_PROTOCOL_MISSING_PREFIX_LEN,         STR_TSP_PROTO_MISSING_PREFIX_LEN },
  { TSP_PROTOCOL_REQ_IN_PROGRESS,            STR_TSP_PROTO_REQ_IN_PROGRESS },
  { TSP_PROTOCOL_PREFIX_REQ_FOR_ANONYMOUS,   STR_TSP_PROTO_PREFIX_REQ_FOR_ANONYMOUS },
  { TSP_PROTOCOL_SERVER_TOO_BUSY,            STR_TSP_PROTO_SERVER_TOO_BUSY },
  { TSP_PROTOCOL_REDIRECT,                   STR_TSP_PROTO_REDIRECT },
  { 0, NULL }
};

// --------------------------------------------------------------------------
// Global static variables used throughout program.
gogocStatusInfo gStatusInfo;       // Declared `extern' in gogoc_c_wrapper.h
gogocTunnelInfo gTunnelInfo;       // Declared `extern' in gogoc_c_wrapper.h
char* gszBrokerListFile = NULL;   // Local only. NOT USED
HACCESSStatusInfo gHACCESSStatusInfo;   // Declared `extern' in gogoc_c_wrapper.h

// --------------------------------------------------------------------------
// Local function prototypes:
void                InitNetToolsArray     ( net_tools_t nt_array[] );
sint32_t            InitLogSystem         ( const tConf* p_config );
void                tspLogOSInfo          ( void );
char *              tspAddPayloadString   ( tPayload *, char * );
gogoc_status         tspUpdateSourceAddr   ( tConf *Conf, pal_socket_t fd );
gogoc_status         tspTunnelNegotiation  ( pal_socket_t socket, tTunnel *t,
                                            tConf *conf, net_tools_t* nt,
                                            sint32_t version_index,
                                            tBrokerList **broker_list );
gogoc_status         tspSetupTunnel        ( tConf *, net_tools_t *,
                                            sint32_t version_index,
                                            tBrokerList **broker_list );


// --------------------------------------------------------------------------
// Call the XML parser. Data will be contained in the structure t.
//
sint32_t tspExtractPayload(char *Payload, tTunnel *t)
{
  char *p;
  sint32_t   rc;

  memset(t, 0, sizeof(tTunnel));

  Display(LOG_LEVEL_3, ELInfo, "tspExtractPayload", STR_MISC_PROCESS_SERVER_REPLY);
  if((p = strchr(strchr(Payload, '\n'), '<')) == NULL)
    return 1;
  if((rc = tspXMLParse(p, t)) != 0)
    return 1;

  return(0);
}


// --------------------------------------------------------------------------
// tspGetStatusCode: Converts the tsp status found at the beginning of the
//   payload to a signed integer. If the payload is NULL, 0 is returned.
//
sint32_t tspGetStatusCode(char *payload)
{
  return (payload!=NULL) ? atoi(payload) : 0;
}


// --------------------------------------------------------------------------
// tspGetTspStatusStr:
//
const char* tspGetTspStatusStr( sint32_t tsp_status )
{
  uint32_t i;

  for(i=0; TSP_PROTOCOL_CODES_STR[i].status != 0; i++)
  {
    if( TSP_PROTOCOL_CODES_STR[i].status == tsp_status )
      return TSP_PROTOCOL_CODES_STR[i].status_str;
  }

  return NULL;
}


// --------------------------------------------------------------------------
// tspAddPayloadString:
//
char *tspAddPayloadString(tPayload *Payload, char *Addition)
{
  char *NewPayload;

  if(Addition) {
    if(Payload->PayloadCapacity == 0)
    {
      if((NewPayload = Payload->payload = (char *)pal_malloc(PROTOCOLMAXPAYLOADCHUNK)) == NULL)
      {
        Display(LOG_LEVEL_1, ELError, "tspAddPayloadString", STR_GEN_MALLOC_ERROR);
        return NULL;
      }
      *Payload->payload = 0;
      Payload->PayloadCapacity = PROTOCOLMAXPAYLOADCHUNK;
    }

    if((Payload->size + (long)pal_strlen(Addition) + 1) > Payload->PayloadCapacity)
    {
      Payload->PayloadCapacity += PROTOCOLMAXPAYLOADCHUNK;
      if((NewPayload = (char *) pal_malloc(Payload->PayloadCapacity)) == NULL)
      {
        Display(LOG_LEVEL_1, ELError, "tspAddPayloadString", STR_GEN_MALLOC_ERROR);
        return NULL;
      }

      memcpy(NewPayload, Payload->payload, Payload->size + 1);
      pal_free(Payload->payload);
      Payload->payload = NewPayload;
    }

    strcat(Payload->payload, Addition);
    Payload->size += pal_strlen(Addition);
  }

  return Payload->payload;
}


// -----------------------------------------------------------------------
// Function: tspUpdateSourceAddr
//
// Description:
//   Retrieves the source IP addres to use for the TSP session.
//
// Arguments:
//   pConf: tConf* [OUT], The global configuration object.
//
// Return Values:
//   gogoc_status value.
//
// -----------------------------------------------------------------------
gogoc_status tspUpdateSourceAddr( tConf *pConf, pal_socket_t fd )
{
  if( pConf->tunnel_mode != V4V6 && pConf->tunnel_mode != DSLITE )
  {
    // tunnel mode v6*v4, we need source IPv4 address
    if( pConf->client_v4 != NULL  &&  pal_strlen(pConf->client_v4) != 0 )
    {
      if( pal_strcasecmp(pConf->client_v4, "auto") == 0 )
      {
        char addr_str[INET_ADDRSTRLEN+1];

        // if current transport is v4, get source address of tsp session. If not, get host address.
        if (((pConf->transport == NET_TOOLS_T_RUDP) || (pConf->transport == NET_TOOLS_T_TCP)) &&
           (tspGetLocalAddress(fd, addr_str, INET_ADDRSTRLEN) != NULL))
        {
          pConf->client_v4 = pal_strdup(addr_str);
          Display(LOG_LEVEL_3, ELInfo, "tspUpdateSourceAddr", STR_NET_USING_SOURCE_IPV4, pConf->client_v4);
        }
        else
        {
          Display(LOG_LEVEL_1, ELError, "tspUpdateSourceAddr", STR_NET_FAILED_FIND_LOCALHOST_IPV4);
          return make_status(CTX_UNSPECIFIED, ERR_INVAL_CLIENT_ADDR);
        }
      }
    }
  }

#ifdef V4V6_SUPPORT
  if( pConf->tunnel_mode == V4V6
#ifdef DSLITE_SUPPORT
    || pConf->tunnel_mode == DSLITE
#endif  
    )
  {
    // tunnel mode v4v6, we need source IPv6 address.
    if( pConf->client_v6 != NULL  &&  pal_strlen(pConf->client_v6) != 0 )
    {
      if( pal_strcasecmp(pConf->client_v6, "auto") == 0 )
      {
        char addr_str[INET6_ADDRSTRLEN+1];

        // if current transport is v6, get source address of tsp session. If not, get host address.
        if (((pConf->transport == NET_TOOLS_T_RUDP6) || (pConf->transport == NET_TOOLS_T_TCP6)) &&
            (tspGetLocalAddress(fd, addr_str, INET6_ADDRSTRLEN) != NULL))
        {
          pConf->client_v6 = pal_strdup(addr_str);
          Display(LOG_LEVEL_3, ELInfo, "tspUpdateSourceAddr", STR_NET_USING_SOURCE_IPV6, pConf->client_v6);
        }
        else
        {
          Display(LOG_LEVEL_1, ELError, "tspUpdateSourceAddr", STR_NET_FAILED_FIND_LOCALHOST_IPV6);
          return make_status(CTX_UNSPECIFIED, ERR_INVAL_CLIENT_ADDR);
        }
      }
      else
      {
        // remove brackets if any
        char addr_str[INET6_ADDRSTRLEN+1];
        char *ptr;

        memset(addr_str, 0, sizeof(addr_str));
        strcpy(addr_str, pConf->client_v6);

        if((ptr = strtok(addr_str, "[")) != NULL)
        {
          if((ptr = strtok(ptr, "]")) != NULL)
          {
            // copy back address without brackets
            strcpy(pConf->client_v6, ptr);
          }
        }
      }
    }
  }
#endif // V4V6_SUPPORT

  return make_status(CTX_UNSPECIFIED, SUCCESS);
}


// --------------------------------------------------------------------------
// tspTunnelNegotiation: Builds and sends a tunnel request to the server.
//   The server will then offer a tunnel. The tunnel settings will be put
//   in the tunnel structure(t).
//
gogoc_status tspTunnelNegotiation( pal_socket_t socket, tTunnel *tunnel_info, tConf *conf, net_tools_t* nt, sint32_t version_index, tBrokerList **broker_list )
{
  tPayload plin;
  tPayload plout;
  sint32_t tsp_status;
  sint32_t ret;


  memset(&plin, 0, sizeof(plin));
  memset(&plout, 0, sizeof(plout));

  // Prepare TSP tunnel request.
  plin.payload = tspAddPayloadString(&plin, tspBuildCreateRequest(conf));

  // Prepare receive buffer.
  plout.size = REDIRECT_RECEIVE_BUFFER_SIZE;
  plout.payload = (char *)pal_malloc(REDIRECT_RECEIVE_BUFFER_SIZE);
  if( plout.payload == NULL )
  {
    return make_status(CTX_TSPTUNNEGOTIATION, ERR_MEMORY_STARVATION);
  }
  memset(plout.payload, 0, plout.size);

  // Send TSP tunnel request over to the server.
  ret = tspSendRecv(socket, &plin, &plout, nt);
  if(ret <= 0)
  {
    pal_free(plout.payload);
    pal_free(plin.payload);
    plout.size = plin.size = 0;

    Display(LOG_LEVEL_1, ELError, "tspTunnelNegotiation", STR_NET_FAIL_RW_SOCKET);
    return make_status(CTX_TSPTUNNEGOTIATION, ERR_SOCKET_IO);
  }

  // Process tunnel reply from server.
  tsp_status = tspGetStatusCode(plout.payload);
  if( tspIsRedirectStatus(tsp_status) )
  {
    if( tspHandleRedirect(plout.payload, conf, broker_list) == TSP_REDIRECT_OK )
    {
      pal_free(plout.payload);
      pal_free(plin.payload);
      plout.size = plin.size = 0;

      return make_status(CTX_TSPTUNNEGOTIATION, EVNT_BROKER_REDIRECTION);
    }
    else
    {
      pal_free(plout.payload);
      pal_free(plin.payload);
      plout.size = plin.size = 0;

      return make_status(CTX_TSPTUNNEGOTIATION, ERR_BROKER_REDIRECTION);
    }
  }
  else if( tsp_status != TSP_PROTOCOL_SUCCESS )
  {
    Display(LOG_LEVEL_1, ELError, "tspTunnelNegotiation", STR_TSP_GEN_ERROR, tsp_status, tspGetTspStatusStr(tsp_status) );
    return make_status(CTX_TSPTUNNEGOTIATION, ERR_TSP_GENERIC_ERROR);
  }

  // Extract the tunnel information from the XML payload.
  tspExtractPayload(plout.payload, tunnel_info);

  // free some memory
  pal_free(plout.payload);
  pal_free(plin.payload);
  plout.size = plin.size = 0;

  // Version 1.0.1 requires that we immediatly jump in tunnel mode.
  // No need to acknowledge the tunnel offered by server.
  if( version_index == CLIENT_VERSION_INDEX_1_0_1 )
  {
    // Successful operation.
    return make_status(CTX_TSPTUNNEGOTIATION, SUCCESS);
  }

  // Acknowledge TSP tunnel offer to server.
  memset(&plin, 0, sizeof(plin));
  plin.payload = tspAddPayloadString(&plin, tspBuildCreateAcknowledge());
  if( tspSend(socket, &plin, nt) == -1 )
  {
    Display(LOG_LEVEL_1, ELError, "tspTunnelNegotiation", STR_NET_FAIL_W_SOCKET);
    return make_status(CTX_TSPTUNNEGOTIATION, ERR_SOCKET_IO);
  }

  // Free the last of the memory
  pal_free(plin.payload);
  plin.size=0;

  // Successful operation.
  return make_status(CTX_TSPTUNNEGOTIATION, SUCCESS);
}


// --------------------------------------------------------------------------
// Attempts to negotiate and setup a tunnel with the broker.
//
gogoc_status tspSetupTunnel(tConf *conf, net_tools_t* nt, sint32_t version_index, tBrokerList **broker_list)
{
  pal_socket_t socket;
  tCapability cap;
  tTunnel tunnel_params;
  gogoc_status status = STATUS_SUCCESS_INIT;


  // Print the transport protocol we're using to perform the TSP session.
  switch( conf->transport )
  {
  case NET_TOOLS_T_TCP:
    Display( LOG_LEVEL_2, ELInfo, "tspSetupTunnel", STR_NET_CONNECT_TCP, conf->server); break;
  case NET_TOOLS_T_RUDP:
    Display( LOG_LEVEL_2, ELInfo, "tspSetupTunnel", STR_NET_CONNECT_RUDP, conf->server); break;
  case NET_TOOLS_T_TCP6:
    Display( LOG_LEVEL_2, ELInfo, "tspSetupTunnel", STR_NET_CONNECT_TCPV6, conf->server); break;
  case NET_TOOLS_T_RUDP6:
    Display( LOG_LEVEL_2, ELInfo, "tspSetupTunnel", STR_NET_CONNECT_RUDPV6, conf->server); break;
  }

  // -----------------------------------------------------------
  // Send(update) connectivity status to GUI.
  // -----------------------------------------------------------
  gStatusInfo.eStatus = GOGOC_CLISTAT__CONNECTING;
  gStatusInfo.nStatus = GOGOCM_UIS__NOERROR;
  send_status_info();


  // ----------------------------------------------------------------------
  // Perform connection to the server using a specific transport protocol.
  // ----------------------------------------------------------------------
  {
    char *srvname;

    // Parse the server name and server port from 'conf->server'.
    if( parse_addr_port( conf->server, &srvname, &conf->port_remote_v4, SERVER_PORT ) != 0 )
    {
      // Bad server name, or bad port.
      return make_status(CTX_NETWORKCONNECT, ERR_INVAL_GOGOC_ADDRESS);
    }

    // Create socket and connect.
    status = tspConnect( &socket, srvname, conf->port_remote_v4, nt );
    if( status_number(status) != SUCCESS )
    {
      Display(LOG_LEVEL_1, ELError, "tspSetupTunnel", STR_NET_FAIL_CONNECT_SERVER, srvname, conf->port_remote_v4);
      pal_free( srvname );
      return status;
    }
    pal_free( srvname );
  }
  if( conf->transport == NET_TOOLS_T_TCP || conf->transport == NET_TOOLS_T_TCP6 )
  {
    // Only display the 'Connected' message when we're using TCP or TCPv6.
    // Because it would be misleading to say we're connected when we're using RUDP/RUDPv6.
    Display(LOG_LEVEL_2, ELInfo, "tspSetupTunnel", STR_GEN_CONNECTED_SERVER, conf->server);
  }
  Display( LOG_LEVEL_3, ELInfo, "tspSetupTunnel", STR_GEN_USING_TSP_PROTO_VER, TSPProtoVerStr[version_index]);

#ifdef DSLITE_SUPPORT
  if (conf->tunnel_mode != DSLITE)
  {
#endif

  // --------------------------------------------
  // Get the TSP capabilities from the server.
  // --------------------------------------------
  Display(LOG_LEVEL_3, ELInfo, "tspSetupTunnel", STR_TSP_GETCAPABILITIES_FROM_SERVER);
  status = tspGetCapabilities(socket, nt, &cap, version_index, conf, broker_list);
  switch( status_number(status) )
  {
  case SUCCESS:
    break;

  case EVNT_BROKER_REDIRECTION: // The redirection event is processed in the main loop.
    tspClose( socket, nt );
    return status;

  case ERR_SOCKET_IO:
    // If the transport is RUDP or RUDPv6, in means we failed to reach a TSP listener.
    // Else, if the transport is TCP or TCPv6, then it's a socket IO error (because we're connected).
    if( conf->transport == NET_TOOLS_T_RUDP || conf->transport == NET_TOOLS_T_RUDP6 )
    {
      Display(LOG_LEVEL_1, ELError, "tspSetupTunnel", STR_TSP_NO_LISTENER, conf->server);
      status = make_status(status_context(status), ERR_FAIL_SOCKET_CONNECT);
    }
    else
      Display(LOG_LEVEL_1, ELError, "tspSetupTunnel", STR_NET_FAIL_RW_SOCKET);
    // *** FALLTHROUGH ***

  default:
    tspClose( socket, nt );
    return status;
  }

  // Verify if the requested tunnel mode is offered on the server.
  if( (conf->tunnel_mode & cap) == 0 )
  {
    // Error: Tunnel mode not supported on server.
    Display(LOG_LEVEL_1, ELError, "tspSetupTunnel", STR_TSP_TUNMODE_NOT_AVAILABLE, conf->server);
    tspClose( socket, nt );
    return make_status(CTX_TSPCAPABILITIES, ERR_TUNMODE_NOT_AVAILABLE);
  }


  // --------------------------------------------
  // Perform TSP authentication on the server.
  // --------------------------------------------
  Display(LOG_LEVEL_3, ELInfo, "tspSetupTunnel", STR_TSP_AUTHENTICATING);
  status = tspAuthenticate(socket, cap, nt, conf, broker_list, version_index);
  switch( status_number(status) )
  {
  case SUCCESS:
    break;

  case EVNT_BROKER_REDIRECTION: // The redirection event is processed in the main loop.
    tspClose( socket, nt );
    return status;

  default:  // An error occurred. The error has already been logged.
    tspClose( socket, nt );
    return status;
  }
  Display(LOG_LEVEL_2, ELInfo, "tspSetupTunnel", STR_TSP_AUTH_SUCCESSFUL);


  // -------------------------------------------------------------------
  // Get the source address to use for local tunnel endpoint.
  // If it was set to 'auto', the source address of the socket is used.
  // -------------------------------------------------------------------
  status = tspUpdateSourceAddr(conf, socket);
  if( status_number(status) != SUCCESS )
  {
    Display(LOG_LEVEL_1, ELError, "tspSetupTunnel", STR_NET_CANT_GET_SRC_ADDRESS);
    tspClose( socket, nt );
    return status;
  }


  // ----------------------------------------------------------------
  // Build and send TSP tunnel request. Then, get tunnel parameters.
  // ----------------------------------------------------------------
  Display(LOG_LEVEL_3, ELInfo, "tspSetupTunnel", STR_TSP_NEGOTIATING_TUNNEL);
  status = tspTunnelNegotiation( socket, &tunnel_params, conf, nt, version_index, broker_list );
  switch( status_number(status) )
  {
  case SUCCESS:
    break;

  case EVNT_BROKER_REDIRECTION: // The redirection event is processed in the main loop.
    tspClose( socket, nt );
    return status;

  default:
    Display(LOG_LEVEL_1, ELError, "tspSetupTunnel", STR_TSP_TUNNEL_NEGO_FAILED, status);
    tspClose( socket, nt );
    return status;
  }
  Display(LOG_LEVEL_2, ELInfo, "tspSetupTunnel", STR_TSP_TUNNEL_NEGO_SUCCESSFUL);

#ifdef DSLITE_SUPPORT
  }
  else
  {
      memset(&tunnel_params, 0, sizeof(tunnel_params));
      tunnel_params.type = pal_strdup(STR_CONFIG_TUNNELMODE_V4V6);
      tunnel_params.lifetime = pal_strdup("0");
      tunnel_params.keepalive_interval = pal_strdup("0");

      status = tspUpdateSourceAddr(conf, socket);
      if( status_number(status) != SUCCESS )
      {
        Display(LOG_LEVEL_1, ELError, "tspSetupTunnel", STR_NET_CANT_GET_SRC_ADDRESS);
        tspClose( socket, nt );
        return status;
      }

      {
          char addr_str[INET6_ADDRSTRLEN+1];
          
          // if current transport is v6, get source address of tsp session. If not, get host address.
          if (tspGetRemoteAddress(socket, addr_str, INET6_ADDRSTRLEN) == NULL)
          {
              Display(LOG_LEVEL_1, ELError, "tspSetupTunnel", STR_NET_CANT_GET_DST_ADDRESS);
              tspClose( socket, nt );
              return make_status(CTX_UNSPECIFIED, ERR_INVAL_GOGOC_ADDRESS);
          }
      
          tunnel_params.server_address_ipv6 = pal_strdup(addr_str);
      }
      
      tunnel_params.client_address_ipv6 = pal_strdup(conf->client_v6);

      tunnel_params.server_address_ipv4 = pal_strdup(conf->dslite_server);
      tunnel_params.client_address_ipv4 = pal_strdup(conf->dslite_client);
  }
#endif
  

  // -------------------------------------------------------------------
  // Save the current server address to the last-tsp-server.txt file.
  // -------------------------------------------------------------------
  if( tspWriteLastServerToFile(conf->last_server_file, conf->server) != TSP_REDIRECT_OK )
  {
    Display(LOG_LEVEL_1, ELError, "tspSetupTunnel", STR_MISC_FAIL_WRITE_LAST_SERVER, conf->server, conf->last_server_file);
  }


  // -------------------------------------------------------
  // Run tunnel script and maintain keepalive (if enabled).
  // -------------------------------------------------------
  status = tspStartLocal(socket, conf, &tunnel_params, nt);


  // -------------------------------------------
  // Clear tunnel information and close socket.
  // -------------------------------------------
  pal_free( gTunnelInfo.szDelegatedPrefix );
  memset( &gTunnelInfo, 0x00, sizeof(struct __TUNNEL_INFO) );
  tspClearTunnelInfo( &tunnel_params );
  tspClose(socket, nt);

  return status;
}


// --------------------------------------------------------------------------
// Function : RetrieveStatusInfo
//
// Description:
//   Will set the status info in ppStatusInfo.
//
// Arguments:
//   ppStatusInfo: gogocStatusInfo** [IN,OUT], The status info.
//
// Return values:
//   GOGOCM_UIS__NOERROR: Successfully retrieved status info and populated the
//                       Status Info object.
//
// --------------------------------------------------------------------------
error_t RetrieveStatusInfo( gogocStatusInfo** ppStatusInfo )
{
  assert( *ppStatusInfo == NULL );

  // No allocation is made, really, we're just assigning the global variable.
  *ppStatusInfo = &gStatusInfo;

  return GOGOCM_UIS__NOERROR;
}

// --------------------------------------------------------------------------
// Function : RetrieveTunnelInfo
//
// Description:
//   Will set the tunnel info in ppTunnelInfo.
//
// Arguments:
//   ppTunnelInfo: gogocTunnelInfo** [IN,OUT], The tunnel info.
//
// Return values:
//   GOGOCM_UIS__NOERROR: Successfully retrieved tunnel info and populated the
//                       Tunnel Info object.
//
// --------------------------------------------------------------------------
error_t RetrieveTunnelInfo( gogocTunnelInfo** ppTunnelInfo )
{
  assert( *ppTunnelInfo == NULL );

  // No allocation is made, really, we're just assigning the global variable.
  *ppTunnelInfo = &gTunnelInfo;

  return GOGOCM_UIS__NOERROR;
}

// --------------------------------------------------------------------------
// Function : RetrieveBrokerList
//
// Description:
//   Will set the broker list in ppBrokerList.
//
// Arguments:
//   ppBrokerList: gogocBrokerList** [IN,OUT], The tunnel info.
//
// Return values:
//   GOGOCM_UIS__NOERROR: Successfully retrieved broker list and populated the
//                       Broker List object.
//   GOGOCM_UIS_FAILEDBROKERLISTEXTRACTION: Failed broker list extraction.
//
// --------------------------------------------------------------------------
error_t RetrieveBrokerList( gogocBrokerList** ppBrokerList )
{
  tBrokerList* tspBrokerList = NULL;// Local format of broker list
  gogocBrokerList* pList;            // Intermediate var
  assert( *ppBrokerList == NULL );  // Allocation is made here.


  // Read broker list from file.
  //
  if( gszBrokerListFile != NULL  &&  tspReadBrokerListFromFile( gszBrokerListFile,
                                       &tspBrokerList ) != TSP_REDIRECT_OK )
  {
    // Failed to extract broker list.
    return GOGOCM_UIS_FAILEDBROKERLISTEXTRACTION;
  }


  // Convert contents.
  if( tspBrokerList != NULL )
  {
    *ppBrokerList = pList = (gogocBrokerList*)pal_malloc( sizeof(gogocBrokerList) );


    // Copy the contents of the broker list struct to the
    // messaging subsystem broker list format.
    //
    while( tspBrokerList != NULL )
    {
      pList->szAddress = pal_strdup(tspBrokerList->address);
      pList->nDistance = tspBrokerList->distance;

      if( (tspBrokerList = tspBrokerList->next) != NULL )
        pList = (pList->next = (gogocBrokerList*)pal_malloc( sizeof(gogocBrokerList) ));
      else
        pList->next = NULL;
    }
  }

  return GOGOCM_UIS__NOERROR;
}

// --------------------------------------------------------------------------
// Function : RetrieveHACCESSStatusInfo
//
// Description:
//   Will set the global HACCESS Status Info object in ppHACCESSStatusInfo.
//
// Arguments:
//   ppHACCESSStatusInfo: HACCESSStatusInfo** [IN,OUT], The HACCESS Status info
//                     returned to the GUI.
//
// Return values:
//   GOGOCM_UIS__NOERROR: Successfully retrieved the HACCESS status info.
//
// --------------------------------------------------------------------------
error_t RetrieveHACCESSStatusInfo( HACCESSStatusInfo** ppHACCESSStatusInfo )
{
#ifdef HACCESS

  HACCESSStatusInfo *haccess_status_info_copy = NULL;
  haccess_status status = HACCESS_STATUS_OK;

  haccess_status_info_copy = (HACCESSStatusInfo *)pal_malloc(sizeof(HACCESSStatusInfo));

  if (haccess_status_info_copy == NULL)
  {
    return GOGOCM_UIS_ERRMEMORYSTARVATION;
  }

  // Ask the HACCESS module for a copy of the current status information.
  status = haccess_messaging_get_status_info(&haccess_status_info_copy);

  if (status != HACCESS_STATUS_OK)
  {
    FreeHACCESSStatusInfo(&haccess_status_info_copy);

    return GOGOCM_UIS_ERRUNKNOWN;
  }

  *ppHACCESSStatusInfo = haccess_status_info_copy;

  return GOGOCM_UIS__NOERROR;
#else
  // Not supposed to happen if HACCESS is not enabled
  return GOGOCM_UIS_ERRUNKNOWN;
#endif
}

// --------------------------------------------------------------------------
void FreeStatusInfo( gogocStatusInfo** ppStatusInfo )
{
  if( *ppStatusInfo != NULL )
  {
    // Nothing is really freed, because we're using the global variable.
    *ppStatusInfo = NULL;
  }
}

// --------------------------------------------------------------------------
void FreeTunnelInfo( gogocTunnelInfo** ppTunnelInfo )
{
  if( *ppTunnelInfo != NULL )
  {
    // Nothing is really freed, because we're using the global variable.
    *ppTunnelInfo = NULL;
  }
}

// --------------------------------------------------------------------------
void FreeBrokerList( gogocBrokerList** ppBrokerList )
{
  gogocBrokerList* pList = *ppBrokerList;

  while( *ppBrokerList != NULL )
  {
    pList = *ppBrokerList;  // Keep reference for free()
    pal_free( pList->szAddress );
    *ppBrokerList = pList->next;
    pal_free( pList );
  }
}

// --------------------------------------------------------------------------
void FreeHACCESSStatusInfo( HACCESSStatusInfo** ppHACCESSStatusInfo )
{
  PMAPPING_STATUS pList = (*ppHACCESSStatusInfo)->haccess_devmap_statuses;

  while ( (*ppHACCESSStatusInfo)->haccess_devmap_statuses != NULL)
  {
    pList = (*ppHACCESSStatusInfo)->haccess_devmap_statuses;
    pal_free( pList->device_name );
    (*ppHACCESSStatusInfo)->haccess_devmap_statuses = pList->next;
    pal_free( pList );
  }

  pal_free(*ppHACCESSStatusInfo);

  *ppHACCESSStatusInfo = NULL;
}

// --------------------------------------------------------------------------
// Function : NotifyhaccessConfigInfo
//
// Description:
//   CALLBACK function from the Messaging Subsystem upon reception of a
//   HACCESS Config Info message.
//
// Arguments:
//   aHACCESSConfigInfo: HACCESSConfigInfo* [IN], The HACCESS Config Info from the GUI.
//
// Return values:
//   GOGOCM_UIS__NOERROR: Successfully processed the HACCESS Config info.
//
// --------------------------------------------------------------------------
error_t NotifyhaccessConfigInfo( const HACCESSConfigInfo* aHACCESSConfigInfo )
{
  return GOGOCM_UIS__NOERROR;
}


// --------------------------------------------------------------------------
// Function: FormatBrokerListAddr
//
// Description: Copies a tBrokerList element address to a destination
//              address. If the address is IPv6, a set of brackets are put
//              at the beginning and end.
// i.e.:
//        1.2.3.4            stays the same
//        broker.domain.com  stays the same
//        2001:BABA::0045    becomes [2001:BABA::0045]
//
// Arguments:
//   listElement: An element of the broker redirection list.
//   ppAddr: A pointer to a char array.
//
// Returns 0 on success, 1 on error.
//
int FormatBrokerListAddr( tBrokerList* listElement, char **ppAddr )
{
  if( ppAddr == NULL )
  {
    // Invalid pointer.
    Display(LOG_LEVEL_1, ELError, "FormatBrokerListAddr", STR_GEN_INVALID_POINTER);
    return 1;
  }

  // Check if there's stuff pointed by ppAddr. Free it if so.
  if( *ppAddr != NULL )
  {
    pal_free( *ppAddr );
    *ppAddr = NULL;
  }

  // Copy address.
  if( listElement->address_type == TSP_REDIRECT_BROKER_TYPE_IPV6 )
  {
    size_t len = pal_strlen(listElement->address) + 3;

    // Must put address between brackets ([]).
    *ppAddr = (char*)pal_malloc( len );
    if( *ppAddr == NULL )
    {
      // Memory allocation error
      Display(LOG_LEVEL_1, ELError, "FormatBrokerListAddr", STR_GEN_MALLOC_ERROR);
      return 1;
    }

    pal_snprintf( *ppAddr, len, "[%s]", listElement->address );
  }
  else
  {
    // Not an IPv6 address, just strdup' it.
    *ppAddr = pal_strdup(listElement->address);
  }

  return 0;
}


// --------------------------------------------------------------------------
// gogoCLIENT TSP main entry point.
// Called from every platform main() or service_main().
//
sint32_t tspMain(sint32_t argc, char *argv[])
{
  tConf c;
  sint32_t log_display_ok = 0;        // Don't use 'Display()'.
  tBrokerList *broker_list = NULL;
  sint32_t trying_original_server = 0;
  sint32_t read_last_server = 0;
  char original_server[MAX_REDIRECT_ADDRESS_LENGTH];
  char last_server[MAX_REDIRECT_ADDRESS_LENGTH];
  tRedirectStatus last_server_status = TSP_REDIRECT_OK;
  gogoc_status status;
  sint32_t loop_delay;

  // Initialize status info.
  gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDIDLE;
  gStatusInfo.nStatus = GOGOCM_UIS__NOERROR;

  // ------------------------------------------------------------------------
  // Zero-memory the configuration object, because tspInitialize requires
  // it initialized. Then call the tspInitialize function.
  // ------------------------------------------------------------------------
  memset( &c, 0, sizeof(c) );
  status = tspInitialize(argc, argv, &c);
  if( status_number(status) != SUCCESS )
  {
    gStatusInfo.nStatus = GOGOCM_UIS_ERRCFGDATA;
    goto endtspc;
  }

  // Initialize the logging system.
  if( InitLogSystem( &c ) != 0 )
  {
    // Failed to allocate memory for the log configuration, or an error
    // happened.
    status = make_status(CTX_CFGVALIDATION, ERR_FAIL_LOG_INIT);

    gStatusInfo.nStatus = GOGOCM_UIS_ERRCFGDATA;
    goto endtspc;
  }
  log_display_ok = 1;

  // Log the OS information through the log system.
  tspLogOSInfo();

  // Keep track of the broker list.
  gszBrokerListFile = c.broker_list_file; // For BROKER_LIST gogocmessaging message.

  // Save the original server value.
  strcpy(original_server, c.server);


  // If always_use_same_server is enabled.
  if( (c.always_use_same_server == TRUE) && (pal_strlen(c.last_server_file) > 0) )
  {
    // Try to get the last server from the last_server file.
    last_server_status = tspReadLastServerFromFile(c.last_server_file, last_server);

    switch( last_server_status )
    {
    case TSP_REDIRECT_OK:
      // Replace the configuration file's server value with the last server.
      pal_free(c.server);
      c.server = pal_strdup(last_server);
      // We found the last server.
      read_last_server = 1;
      // We're not trying the original server.
      trying_original_server = 0;
      Display(LOG_LEVEL_2, ELInfo, "tspMain", GOGO_STR_RDR_TRYING_LAST_SERVER, last_server);
      break;

    case TSP_REDIRECT_NO_LAST_SERVER:
      // Try the original server instead.
      trying_original_server = 1;
      Display(LOG_LEVEL_2, ELInfo, "tspMain", GOGO_STR_RDR_NO_LAST_SERVER_FOUND, c.last_server_file, original_server);
      break;

    case TSP_REDIRECT_CANT_OPEN_FILE:
      // Try the original server instead.
      trying_original_server = 1;
      Display(LOG_LEVEL_2, ELInfo, "tspMain", GOGO_STR_RDR_CANT_OPEN_LAST_SERVER, c.last_server_file, original_server);
      break;

    default:
      Display(LOG_LEVEL_1, ELError, "tspMain", GOGO_STR_RDR_ERROR_READING_LAST_SERVER, c.last_server_file);
      status = make_status(CTX_CFGVALIDATION, ERR_FAIL_LAST_SERVER);

      gStatusInfo.nStatus = GOGOCM_UIS_ERRBROKERREDIRECTION;
      goto endtspc;
    }
  }
  else
  {
    // If always_use_same_server is disabled, try the original server.
    trying_original_server = 1;
  }

  loop_delay = c.retry_delay;

  do {
  net_tools_t nt[NET_TOOLS_T_SIZE];
  sint32_t version_index = CLIENT_VERSION_INDEX_CURRENT;
  sint32_t connected = 1;             // try servers as long as connected is true */
  sint32_t cycle = 0;                 // reconnect and fallback cycle */
  sint32_t tsp_version_fallback = 0;  // true if the TSP protocol version needs to fall back for the next retry */
  sint32_t quick_cycle = 0;
  tBrokerList *current_broker_in_list = NULL;
  sint32_t trying_broker_list = 0;
  tRedirectStatus broker_list_status = TSP_REDIRECT_OK;
  uint16_t effective_retry_delay;
  uint8_t  consec_retry = 0;

  // Initialize the net tools array.
  memset( nt, 0, sizeof(nt) );
  InitNetToolsArray( nt );



  // ------------------------------------------------------------------------
  // Connection loop.
  //   Perform loop until we give up (i.e.: an error is indicated), or user
  //   requested a stop in the service (HUP signal or service stop).
  //
  while( connected )
  {
    if (tspCheckForStopOrWait(0) != 0)
      goto endtspc;

    // While we loop in this while(), keep everything updated on our status.
    //
    if( gStatusInfo.eStatus != GOGOC_CLISTAT__DISCONNECTEDIDLE &&
        gStatusInfo.nStatus != GOGOCM_UIS__NOERROR )
    {
      // Status has been changed.
      send_status_info();
    }

    // Choose the transport or cycle thru the list
    switch( c.tunnel_mode )
    {
    case V6UDPV4:
      switch( cycle )
      {
        default:
          cycle = 0;  // Catch an overflow of the variable.
          // *** FALLTHROUGH ***

        case 0:
          if( tsp_version_fallback )
          {
            if( version_index < CLIENT_VERSION_INDEX_V6UDPV4_START &&
                version_index < CLIENT_VERSION_INDEX_OLDEST )
            {
              version_index++;
            }
            else
            {
              connected = 0;
              continue;
            }
            tsp_version_fallback = 0;
          }
          else
          {
            version_index = CLIENT_VERSION_INDEX_CURRENT;
          }
          c.transport = NET_TOOLS_T_RUDP;
        break;
      }
      break;

    case V6ANYV4:
    case V6V4:
      switch( cycle )
      {
        default:
          cycle = 0;  // Catch an overflow of the variable.
          // *** FALLTHROUGH ***

        case 0:
          if( tsp_version_fallback )
          {
            if( version_index < CLIENT_VERSION_INDEX_OLDEST )
            {
              version_index++;
            }
            else
            {
              connected = 0;
              continue;
            }
            tsp_version_fallback = 0;
          }
          else
          {
            version_index = CLIENT_VERSION_INDEX_CURRENT;
          }
          c.transport = NET_TOOLS_T_RUDP;
          break;

        case 1:
          if( tsp_version_fallback )
          {
            if( version_index < CLIENT_VERSION_INDEX_OLDEST )
            {
              version_index++;
            }
            else
            {
              connected = 0;
              continue;
            }
            tsp_version_fallback = 0;
          }
          else
          {
            version_index = CLIENT_VERSION_INDEX_CURRENT;
          }
          c.transport = NET_TOOLS_T_TCP;
          break;
      }
      break;

    case V4V6:
#ifdef V4V6_SUPPORT
#ifdef DSLITE_SUPPORT
    case DSLITE:
#endif        
        switch( cycle )
      {
        default:
          cycle = 0;  // Catch an overflow of the variable.
          // *** FALLTHROUGH ***

        case 0:
          if( tsp_version_fallback )
          {
            if( version_index < CLIENT_VERSION_INDEX_V4V6_START &&
                version_index < CLIENT_VERSION_INDEX_OLDEST )
            {
              version_index++;
            }
            else
            {
              connected = 0;
              continue;
            }
            tsp_version_fallback = 0;
          }
          else
          {
            version_index = CLIENT_VERSION_INDEX_CURRENT;
          }
          c.transport = NET_TOOLS_T_RUDP6;
          break;

        case 1:
          if( tsp_version_fallback )
          {
            if( version_index < CLIENT_VERSION_INDEX_V4V6_START &&
                version_index < CLIENT_VERSION_INDEX_OLDEST )
            {
              version_index++;
            }
            else
            {
              connected = 0;
              continue;
            }
            tsp_version_fallback = 0;
          }
          else
          {
            version_index = CLIENT_VERSION_INDEX_CURRENT;
          }
#ifdef DSLITE_SUPPORT
          c.transport = c.tunnel_mode == DSLITE ? NET_TOOLS_T_RUDP6 : NET_TOOLS_T_TCP6;
#else
          c.transport = NET_TOOLS_T_TCP6;
#endif
          break;
      }
#endif
      break;
    } // switch(c.tunnel_mode)


    // Determine if we need to sleep between connection attempts.
    quick_cycle = 0;
    if( ( (c.tunnel_mode == V6ANYV4) || (c.tunnel_mode == V6V4) ) && (c.transport == NET_TOOLS_T_RUDP) )
    {
      quick_cycle = 1;
    }
#ifdef V4V6_SUPPORT
    if( (c.tunnel_mode == V4V6) && (c.transport == NET_TOOLS_T_RUDP6) )
    {
      quick_cycle = 1;
    }
#endif
#ifdef DSLITE_SUPPORT
    if( (c.tunnel_mode == DSLITE) && (c.transport == NET_TOOLS_T_RUDP6) )
    {
      quick_cycle = 1;
    }
#endif
    

    // -----------------------------------------------
    // *** Attempt to negotiate tunnel with broker ***
    // -----------------------------------------------
    status = tspSetupTunnel(&c, &nt[c.transport], version_index, &broker_list);

    switch( status_number(status) )
    {
    case SUCCESS:
      // If we are here with no error, we can assume we are finished.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDIDLE;
        gStatusInfo.nStatus = GOGOCM_UIS__NOERROR;

        connected = 0;
        continue;
      }
      break;

    case ERR_KEEPALIVE_TIMEOUT:
      // A keepalive timeout has occurred.
      {
        Display(LOG_LEVEL_1, ELError, "tspMain", STR_KA_GENERAL_TIMEOUT);

        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRKEEPALIVETIMEOUT;

        consec_retry = 0;
        if( c.auto_retry_connect == FALSE )
        {
          gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDNORETRY;
          connected = 0;
          continue;
        }
      }
      break;

    case ERR_AUTHENTICATION_FAILURE:
      // There's nothing more to do if the authentication has failed.
      // The user needs to change its username/password. Abort.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRAUTHENTICATIONFAILURE;

        connected = 0;
        continue;
      }
      break;

    case ERR_NO_COMMON_AUTHENTICATION:
      // Configured authentication method is not supported by server.
      // User needs to change configuration. Abort.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRNOCOMMONAUTHENTICATION;

        connected = 0;
        continue;
      }
      break;

    case ERR_INTERFACE_SETUP_FAILED:
      // The tunnel interface configuration script failed. Abort.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRINTERFACESETUPFAILED;

        connected = 0;
        continue;
      }
      break;

    case ERR_TUN_LEASE_EXPIRED:
      // The tunnel lease has expired. Reconnect.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDIDLE;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRTUNLEASEEXPIRED;

        Display(LOG_LEVEL_1, ELWarning, "tspMain", STR_TSP_TUNNEL_LEASE_EXPIRED);
        continue;
      }

    case ERR_INVAL_TSP_VERSION:
      // Invalid TSP version used.  Will change version on next connect.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRTSPVERSIONERROR;

        tsp_version_fallback = 1;
        if( version_index == CLIENT_VERSION_INDEX_2_0_0 )
        {
          cycle = 1;
        }

        // Wait a little to prevent the TSP version fallback problem with UDP
        // connections that have the same source port. See Bugzilla bug #3539
        if ((version_index != CLIENT_VERSION_INDEX_2_0_0) && (c.transport == NET_TOOLS_T_RUDP
  #ifdef V4V6_SUPPORT
          || c.transport == NET_TOOLS_T_RUDP6
  #endif
          ))
        {
          pal_sleep(TSP_VERSION_FALLBACK_DELAY);
        }
        Display (LOG_LEVEL_1, ELInfo, "tspMain", STR_GEN_DISCONNECTED_RETRY_NOW);
        continue;
      }
      break;

    case ERR_SOCKET_IO:
      // Socket error. Reconnect, don't change transport.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRSOCKETIO;

        consec_retry = 0;
        if( quick_cycle != 0 )
        {
          Display(LOG_LEVEL_1, ELInfo, "tspMain", STR_GEN_DISCONNECTED_RETRY_NOW);
          continue;
        }
      }
      break;

    case ERR_TSP_SERVER_TOO_BUSY:
      // The server is currently too busy to process the TSP request.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRTSPSERVERTOOBUSY;

        // Force a wait.
        effective_retry_delay = c.retry_delay;
        consec_retry = 1;
      }
      break;

    case ERR_TSP_GENERIC_ERROR:
      // Unexpected TSP status in TSP session. Abort.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRTSPGENERICERROR;

        connected = 0;
        continue;
      }
      break;

    case ERR_FAIL_SOCKET_CONNECT:
      // This means we could not connect to a server.
      // We'll try a different transport to the same server.
      // If that fails too, we'll go through the broker list (if any).
      {
        // Don't retry to avoid blocking the boot
        if (c.boot_mode)
        {
          connected = 0;
          continue;
        }

        if( quick_cycle == 1 )
        {
          // We haven't tried all transports for this broker, there are more to try.
          cycle++;
          Display(LOG_LEVEL_1, ELInfo, "tspMain", STR_GEN_DISCONNECTED_RETRY_NOW);
          continue;
        }

        // If we have the last server, we always need to connect to this one.
        if( read_last_server == 1 )
        {
          // Just cycle transports, try again with the last server.
          cycle++;
          break;
        }

        // Do the following only if we have tried all transports(and failed to connect)
        effective_retry_delay = c.retry_delay;
        consec_retry = 1;
        // Status update.
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRFAILSOCKETCONNECT;

        // If we're trying to connect to the original server.
        if( trying_original_server == 1 )
        {
          // Clear the broker list.
          tspFreeBrokerList(broker_list);
          broker_list = NULL;

          // If a broker_list file is specified, try to create the list
          if( pal_strlen(c.broker_list_file) > 0 )
          {
            Display (LOG_LEVEL_2, ELInfo, "tspMain", GOGO_STR_RDR_READING_BROKER_LIST, c.broker_list_file);

            broker_list_status = tspReadBrokerListFromFile(c.broker_list_file, &broker_list);
            switch( broker_list_status )
            {
            case TSP_REDIRECT_OK:
              // If the broker list is empty.
              if( broker_list == NULL )
              {
                Display (LOG_LEVEL_2, ELInfo, "tspMain", GOGO_STR_RDR_READ_BROKER_LIST_EMPTY);
                // Just cycle transports, we'll try the original server again.
                cycle++;
              }
              // If the broker list is not empty.
              else
              {
                Display (LOG_LEVEL_2, ELInfo, "tspMain", GOGO_STR_RDR_READ_BROKER_LIST_CREATED);

                tspLogRedirectionList(broker_list, 0);

                // We're going through a broker list.
                trying_broker_list = 1;
                // We're not trying the original server anymore.
                trying_original_server = 0;
                // Start with the first broker in the list.
                current_broker_in_list = broker_list;
                // Copy the brokerList address to configuration server.
                if( FormatBrokerListAddr( current_broker_in_list, &(c.server) ) != 0 )
                {
                  tspFreeBrokerList(broker_list);
                  broker_list = NULL;
                  status = make_status( status_context(status), ERR_BROKER_REDIRECTION );
                  gStatusInfo.nStatus = GOGOCM_UIS_ERRBROKERREDIRECTION;
                  connected = 0;
                  goto endtspc;
                }
                // Adjust the transport cycle to start from the first one.
                cycle = 0;
                // Try the first broker in the list right now.
                continue;
              }
              break;

            case TSP_REDIRECT_CANT_OPEN_FILE:
              // If we can't open the file, maybe it's just not there.
              // This is normal if it hasn't been created.
              Display (LOG_LEVEL_2, ELInfo, "tspMain", GOGO_STR_RDR_CANT_OPEN_BROKER_LIST, c.broker_list_file);
              cycle++;
              tspFreeBrokerList(broker_list);
              broker_list = NULL;
              break;

            case TSP_REDIRECT_TOO_MANY_BROKERS:
              // If there were more brokers in the list than the allowed limit.
              Display (LOG_LEVEL_1, ELError, "tspMain", GOGO_STR_RDR_TOO_MANY_BROKERS, MAX_REDIRECT_BROKERS_IN_LIST);
              cycle++;
              tspFreeBrokerList(broker_list);
              broker_list = NULL;
              break;

            default:
              // There was a problem creating the list from the broker_list file
              Display (LOG_LEVEL_1, ELError, "tspMain", GOGO_STR_RDR_ERROR_READING_BROKER_LIST, c.broker_list_file);
              cycle++;
              tspFreeBrokerList(broker_list);
              broker_list = NULL;
              break;
            }
          }
          else
          {
            // Nothing specified in broker_list. Cycle transports, but 
            // try same server again.
            cycle++;
          }
        }
        // If we're not trying to connect to the original server.
        // and we're going through a broker list.
        else if( trying_broker_list == 1 )
        {
          // If the pointers aren't safe.
          if( (broker_list == NULL) || (current_broker_in_list == NULL) )
          {
            Display(LOG_LEVEL_1, ELError, "tspMain", GOGO_STR_RDR_BROKER_LIST_INTERNAL_ERROR, current_broker_in_list->address);
            gStatusInfo.nStatus = GOGOCM_UIS_ERRBROKERREDIRECTION;
            status = make_status( status_context(status), ERR_BROKER_REDIRECTION );

            tspFreeBrokerList(broker_list);
            broker_list = NULL;
            connected = 0;
            continue;
          }

          // If this is the last broker in the list.
          if( current_broker_in_list->next == NULL )
          {
            Display (LOG_LEVEL_2, ELInfo, "tspMain", GOGO_STR_RDR_BROKER_LIST_END);

            // Prepare to retry the original server after the retry delay.
            pal_free(c.server);
            c.server = pal_strdup(original_server);
            cycle = 0;
            trying_original_server = 1;
            break;
          }

          // Prepare to try the next broker in the list.
          current_broker_in_list = current_broker_in_list->next;

          // Copy the brokerList address to configuration server.
          if( FormatBrokerListAddr( current_broker_in_list, &(c.server) ) != 0 )
          {
            tspFreeBrokerList(broker_list);
            broker_list = NULL;
            status = make_status( status_context(status), ERR_BROKER_REDIRECTION );
            gStatusInfo.nStatus = GOGOCM_UIS_ERRBROKERREDIRECTION;
            connected = 0;
            goto endtspc;
          }

          Display(LOG_LEVEL_2, ELInfo, "tspMain", GOGO_STR_RDR_NEXT_IN_BROKER_LIST, current_broker_in_list->address);

          // Try the next broker now, don't wait for the retry delay.
          cycle = 0;
          continue;
        }
      }
      break;

    case EVNT_BROKER_REDIRECTION:
      // This means we got a broker redirection message. The handling
      // function that sent us this signal has created the broker list.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDIDLE;
        gStatusInfo.nStatus = GOGOCM_UIS_EVNTBROKERREDIRECTION;

        // Check that the broker list has been created.
        if( broker_list != NULL )
        {
          // We're going through a broker list.
          trying_broker_list = 1;
          // We're not trying to connect to the original server.
          trying_original_server = 0;
          // Prepare to try the first broker in the list.
          current_broker_in_list = broker_list;
          // Try the first broker in the list without waiting for the retry delay.
          cycle = 0;
          // Copy the brokerList address to configuration server.
          if( FormatBrokerListAddr( current_broker_in_list, &(c.server) ) != 0 )
          {
            // Error: Failed to format the address. Abort.
            gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
            gStatusInfo.nStatus = GOGOCM_UIS_ERRBROKERREDIRECTION;
            status = make_status(status_context(status), ERR_BROKER_REDIRECTION);
            connected = 0;
          }
        }
        else
        {
          // Error: Empty or invalid broker list. Abort.
          gStatusInfo.nStatus = GOGOCM_UIS_ERRBROKERREDIRECTION;
          status = make_status(status_context(status), ERR_BROKER_REDIRECTION);
          connected = 0;
        }
        continue;
      }
      break;

    case ERR_BROKER_REDIRECTION:
      // This means we got a broker redirection message, but there were
      // errors in handling it.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRBROKERREDIRECTION;
        Display(LOG_LEVEL_1, ELError, "tspMain", GOGO_STR_RDR_ERROR_PROCESSING_REDIRECTION, c.server);

        tspFreeBrokerList(broker_list);
        broker_list = NULL;

        connected = 0;
        continue;
      }
      break;

    case ERR_MEMORY_STARVATION:
      // This is a fatal error.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRMEMORYSTARVATION;

        connected = 0;
        continue;
      }
      break;

    case ERR_TUNMODE_NOT_AVAILABLE:
      // Configured tunnel mode is not available on the server.
      // User needs to change configuration. Abort.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRTUNMODENOTAVAILABLE;

        connected = 0;
        continue;
      }
      break;

    case ERR_TUNNEL_IO:
      // Occurs if there is a problem during the tunneling with a TUN interface.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRTUNNELIO;

        consec_retry = 0;
      }
      break;

    case ERR_KEEPALIVE_ERROR:
      // A keepalive error occured. Probably a network error.
      // Since the keepalive error occurs after the TSP session, it is safe
      //   to assume that we can reconnect and try again.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRKEEPALIVEERROR;

        consec_retry = 0;
      }
      break;

    case ERR_BAD_TUNNEL_PARAM:
      // The tunnel information that the server provided was bad.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRBADTUNNELPARAM;

        connected = 0;
        continue;
      }
      break;

#ifdef HACCESS
    case ERR_HACCESS_SETUP:
      {
        Display(LOG_LEVEL_1, ELError, "tspMain", HACCESS_LOG_PREFIX_ERROR GOGO_STR_HACCESS_ERR_FAILED_TO_SETUP_HACCESS_FEATURES);

        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDHACCESSSETUPERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRHACCESSSETUP;

        connected = 0;
        continue;
      }
      break;

    case ERR_HACCESS_EXPOSE_DEVICES:
      // Home access error: Failed to make the devices available. Abort.
      {
        Display(LOG_LEVEL_1, ELError, "tspMain", HACCESS_LOG_PREFIX_ERROR GOGO_STR_HACCESS_ERR_FAILED_TO_EXPOSE_DEVICES);

        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDHACCESSEXPOSEDEVICESERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRHACCESSEXPOSEDEVICES;

        connected = 0;
        continue;
      }
      break;
#endif

    case ERR_INVAL_GOGOC_ADDRESS:
    case ERR_FAIL_RESOLV_ADDR:
      // Failed to parse server Address or resolve it.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRINVALSERVERADDR;

        connected = 0;
        continue;
      }
      break;

    case ERR_INVAL_CFG_FILE:
    case ERR_INVAL_CLIENT_ADDR:
    default:
      // Any other error, quit immediatly.
      {
        gStatusInfo.eStatus = GOGOC_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GOGOCM_UIS_ERRUNKNOWN;

        connected = 0;
        continue;
      }
      break;
    } // status switch


    // Send status info about the current connection failure.
    //
    send_status_info();

    // Do not wait if it is the first reconnection attempt.
    // The delay to wait SHALL be doubled at every 3 consecutive failed
    //   connection attempts. It MUST NOT exceed configured retry_delay_max.
    //
    if( consec_retry > 0 )
    {
      sint32_t sleepTime = c.retry_delay;
      if( consec_retry % CONSEC_RETRY_TO_DOUBLE_WAIT == 0 )
      {
        // Double the effective wait time.
        effective_retry_delay *= 2;
        if( effective_retry_delay > c.retry_delay_max )
          effective_retry_delay = c.retry_delay_max;
      }
      consec_retry++;
      sleepTime = effective_retry_delay;

      // Log connection failure & sleep before retrying.
      Display( LOG_LEVEL_1, ELInfo, "tspMain", STR_GEN_DISCONNECTED_RETRY_SEC, effective_retry_delay );

      // Check for stop at each second.
      while( sleepTime-- > 0  &&  tspCheckForStopOrWait(1000) == 0 );
    }
    else
    {
      consec_retry++;
      effective_retry_delay = c.retry_delay;

      // Log connection failure. Reconnect now.
      Display( LOG_LEVEL_1, ELInfo, "tspMain", STR_GEN_DISCONNECTED_RETRY_NOW );
    }

  }  // Profanely big connection while()

  {
    uint32_t sleepTime = loop_delay;
    while( sleepTime-- > 0  &&  tspCheckForStopOrWait(1000) == 0 );
  }
  loop_delay *= 2;
  if (loop_delay > c.retry_delay_max) loop_delay = c.retry_delay_max;

  } while (!c.boot_mode);

endtspc:
  // Send final status to GUI.
  send_status_info();

  // Free the broker list.
  tspFreeBrokerList(broker_list);

  // Display the last status context, if the status number is not SUCCESS.
  if( status_number(status) != SUCCESS )
  {
    if( log_display_ok == 0 )
      DirectErrorMessage(STR_GEN_LAST_STATUS_CONTEXT, GOGOCStatusContext[status_context(status)]);
    else
      Display(LOG_LEVEL_1, ELWarning, "tspMain", STR_GEN_LAST_STATUS_CONTEXT, GOGOCStatusContext[status_context(status)]);
  }

  // Log end of program.
  if( log_display_ok == 0 )
    DirectErrorMessage(STR_GEN_FINISHED);
  else
    Display(LOG_LEVEL_1, ELInfo, "tspMain", STR_GEN_FINISHED);

  // Close the log system
  LogClose();


  return( status_number(status) );
}


// --------------------------------------------------------------------------
// InitNetToolsArray:
//
// Parameter:
//   nt_array: Pointer to the net tools array to initialize.
//
// Returned values: (none)
//
void InitNetToolsArray( net_tools_t nt_array[] )
{
  // Fill up the array.
  (nt_array)[NET_TOOLS_T_RUDP].netopen = NetRUDPConnect;
  (nt_array)[NET_TOOLS_T_RUDP].netclose = NetRUDPClose;
  (nt_array)[NET_TOOLS_T_RUDP].netsendrecv = NetRUDPReadWrite;
  (nt_array)[NET_TOOLS_T_RUDP].netsend = NetRUDPWrite;
  (nt_array)[NET_TOOLS_T_RUDP].netprintf = NetRUDPPrintf;
  (nt_array)[NET_TOOLS_T_RUDP].netrecv = NetRUDPRead;

  (nt_array)[NET_TOOLS_T_UDP].netopen = NetUDPConnect;
  (nt_array)[NET_TOOLS_T_UDP].netclose = NetUDPClose;
  (nt_array)[NET_TOOLS_T_UDP].netsendrecv = NetUDPReadWrite;
  (nt_array)[NET_TOOLS_T_UDP].netsend = NetUDPWrite;
  (nt_array)[NET_TOOLS_T_UDP].netprintf = NetUDPPrintf;
  (nt_array)[NET_TOOLS_T_UDP].netrecv = NetUDPRead;

  (nt_array)[NET_TOOLS_T_TCP].netopen = NetTCPConnect;
  (nt_array)[NET_TOOLS_T_TCP].netclose = NetTCPClose;
  (nt_array)[NET_TOOLS_T_TCP].netsendrecv = NetTCPReadWrite;
  (nt_array)[NET_TOOLS_T_TCP].netsend = NetTCPWrite;
  (nt_array)[NET_TOOLS_T_TCP].netprintf = NetTCPPrintf;
  (nt_array)[NET_TOOLS_T_TCP].netrecv = NetTCPRead;

#ifdef V4V6_SUPPORT
  (nt_array)[NET_TOOLS_T_TCP6].netopen = NetTCP6Connect;
  (nt_array)[NET_TOOLS_T_TCP6].netclose = NetTCP6Close;
  (nt_array)[NET_TOOLS_T_TCP6].netsendrecv = NetTCP6ReadWrite;
  (nt_array)[NET_TOOLS_T_TCP6].netsend = NetTCP6Write;
  (nt_array)[NET_TOOLS_T_TCP6].netprintf = NetTCP6Printf;
  (nt_array)[NET_TOOLS_T_TCP6].netrecv = NetTCP6Read;

  (nt_array)[NET_TOOLS_T_RUDP6].netopen = NetRUDP6Connect;
  (nt_array)[NET_TOOLS_T_RUDP6].netclose = NetRUDP6Close;
  (nt_array)[NET_TOOLS_T_RUDP6].netsendrecv = NetRUDP6ReadWrite;
  (nt_array)[NET_TOOLS_T_RUDP6].netsend = NetRUDP6Write;
  (nt_array)[NET_TOOLS_T_RUDP6].netprintf = NetRUDP6Printf;
  (nt_array)[NET_TOOLS_T_RUDP6].netrecv = NetRUDP6Read;
#endif // V4V6_SUPPORT
}


// --------------------------------------------------------------------------
// InitLogSystem: Allocates memory for the log configuration parameters and
//   initializes them with the values pass in 'p_config'.
//
// Parameter:
//   p_config: A valid tConf structure containing log configuration info.
//
// Returned values:
//   0 on success
//  -1 on error.
//
sint32_t InitLogSystem( const tConf* p_config )
{
  tLogConfiguration * p_log_config;


  // Check configuration pointer.
  if( p_config == NULL )
  {
    return -1;
  }

  // Allocate memory for the logging configuration structure.
  p_log_config = (tLogConfiguration *)pal_malloc(sizeof(tLogConfiguration));
  if( p_log_config == NULL )
  {
    DirectErrorMessage(STR_GEN_MALLOC_ERROR);
    return -1;
  }

  // Fill the logging configuration structure with the values parsed from the
  //   configuration. It is possible that some of those values are default
  //   values that were automatically set by the client code if the user did
  //   not specify alternative values.
  p_log_config->identity = pal_strdup(LOG_IDENTITY);
  p_log_config->log_filename = pal_strdup(p_config->log_filename);
  p_log_config->log_level_stderr = p_config->log_level_stderr;
  p_log_config->log_level_console = p_config->log_level_console;
  p_log_config->log_level_syslog = p_config->log_level_syslog;
  p_log_config->log_level_file = p_config->log_level_file;
  p_log_config->syslog_facility = p_config->syslog_facility;
  p_log_config->log_rotation = p_config->log_rotation;
  p_log_config->log_rotation_size = p_config->log_rotation_size;
  p_log_config->delete_rotated_log = p_config->log_rotation_delete;
  p_log_config->buffer = 0;

  // Configure the logging system with the values provided above.
  if( LogConfigure(p_log_config) != 0 )
  {
    DirectErrorMessage(STR_MISC_LOG_CONFIGURE_FAILED);
    return -1;
  }

  // Successful operation.
  return 0;
}


// --------------------------------------------------------------------------
// tspLogOSInfo: Logs the OS information through the log system.
//
void tspLogOSInfo( void )
{
  char bufOSInfo[256];

  // Display gogoCLIENT version and build option(s).
  Display( LOG_LEVEL_1, ELInfo, "tspLogOSInfo", "%s", tsp_get_version() );

  // Display OS specific information. Handy for bug reporting.
  tspGetOSInfo( sizeof(bufOSInfo), bufOSInfo );  // in tsp_local.c
  Display( LOG_LEVEL_1, ELInfo, "tspLogOSInfo", "%s", bufOSInfo );
}

#ifdef ANDROID
#include <fcntl.h>
void writepid()
{
#define PIDFILE "/data/data/org.nklog.gogoc/files/gogoc.pid"
#define FLAG (O_WRONLY | O_CREAT | O_TRUNC)
  int runpid;
  int length;
  char buf[sizeof(int)*3 + 2];
  if ((runpid = open(PIDFILE, FLAG, 0666))) {
    length = sprintf((char *)buf, "%d\n", getpid());
    write(runpid, &buf, length);
    close(runpid);
  }
}
#endif
