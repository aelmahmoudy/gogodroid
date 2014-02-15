/*
---------------------------------------------------------------------------
 $Id: tsp_net.c,v 1.1 2009/11/20 16:53:41 jasminko Exp $
---------------------------------------------------------------------------
 Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
---------------------------------------------------------------------------
*/

#include "platform.h"
#include "gogoc_status.h"

#include "tsp_net.h"
#include "net.h"
#include "log.h"
#include "hex_strings.h"


// --------------------------------------------------------------------------
// tspConnect: Creates a socket and connects to the specified server on the
//   specified port.
//
gogoc_status tspConnect( pal_socket_t *p_sock, char *srvname, uint16_t srvport, net_tools_t *nt )
{
  sint32_t ret;
  gogoc_status status = STATUS_SUCCESS_INIT;


  ret = nt->netopen( p_sock, srvname, srvport );
  switch( ret )
  { 
  case 0:
    status = make_status(CTX_NETWORKCONNECT, SUCCESS); break;

  case -1:  // Failed to resolve srvname.
    status = make_status(CTX_NETWORKCONNECT, ERR_FAIL_RESOLV_ADDR); break;

  case -2:  // Failed to connect (TCP only) to srvname.
    status = make_status(CTX_NETWORKCONNECT, ERR_FAIL_SOCKET_CONNECT); break;
  }

  return status;
}


// --------------------------------------------------------------------------
// tspClose:
//
gogoc_status tspClose(pal_socket_t sock, net_tools_t *nt)
{
  sint32_t ret = nt->netclose(sock);

  return make_status( CTX_UNSPECIFIED, (ret==0) ? SUCCESS : ERR_SOCKET_IO );
}


// --------------------------------------------------------------------------
// tspSendRecv: Sends a payload to server and receives reply payload.
//   NOTE: plin is data to be sent and plout->payload is a buffer for data to
//         be received.
//
sint32_t tspSendRecv(pal_socket_t socket, tPayload *plin, tPayload *plout, net_tools_t *nt)
{
  char string[] = "Content-length: %ld\r\n";
  char buffer[PROTOCOLFRAMESIZE];
  char *ptr_b, *ptr_c;
  sint32_t read, ret, size, left;


  // add in content-length to data to be sent.
  pal_snprintf(buffer, PROTOCOLFRAMESIZE, string, plin->size);
  size = pal_strlen(buffer);
  memcpy(buffer + size, plin->payload, plin->size);

  buffer[size + plin->size] = 0;
  Display(LOG_LEVEL_3, ELInfo, "tspSendRecv", STR_NET_SENDING, buffer);

  // Send 'buffer', recv in 'plout->payload'.
  ret = nt->netsendrecv(socket, buffer, size + plin->size, plout->payload, plout->size);
  if( ret <= 0 )
  {
    Display(LOG_LEVEL_1, ELError, "tspSendRecv", STR_NET_FAIL_RW_SOCKET);
    return ret;
  }

  // Validate that we got 'Content-Length'.
  if( memcmp(plout->payload, "Content-length:", 15) )
  {
    Display(LOG_LEVEL_1, ELError, "tspSendRecv", GOGO_STR_EXPECTED_CONTENT_LENGTH, plout->payload);
    return -1;
  }

  /* strip it from the returned string */
  ptr_c = strchr(plout->payload, '\n');

  /* test if valid data received (see bug 3295) */
  if( ptr_c == NULL )
  {
    Display(LOG_LEVEL_1, ELError, "tspSendRecv", GOGO_STR_RECV_INVALID_TSP_DATA);
    return -1;
  }
  ptr_c++;
  size = pal_strlen(ptr_c);

  /* validate received data using Content-Length (see bug: 3164) */
  if( ((plout->size = atol(plout->payload + 15)) <= 0L) || (size > plout->size) )
  {
    Display(LOG_LEVEL_1, ELError, "tspSendRecv", GOGO_STR_INVALID_PAYLOAD_SIZE);
    return -1;
  }

  left = plout->size - size;
  while( left > 0 )
  {
    if( (read = nt->netrecv(socket, (ptr_c + size), left)) <= 0 )
    {
      Display(LOG_LEVEL_1, ELError, "tspSendRecv", STR_NET_FAIL_R_SOCKET);
      return PROTOCOL_ERROR;
    }
    size += read;
    left -= read;
  }

  ptr_b = (char *)pal_malloc(++size); // need space for a little 0 at the end */
  memset(ptr_b, 0, size);
  memcpy(ptr_b, ptr_c, --size);   /* but need not to overwrite that little 0 */

  pal_free(plout->payload);
  plout->payload = ptr_b;

  Display(LOG_LEVEL_3, ELInfo, "tspSendRecv", STR_NET_RECEIVED, plout->payload);

  return ret;
}


// --------------------------------------------------------------------------
// tspSend:
//
sint32_t tspSend(pal_socket_t socket, tPayload *pl, net_tools_t *nt)
{
  char buffer[PROTOCOLFRAMESIZE];
  long ClSize;
  sint32_t ret;

  pal_snprintf(buffer, PROTOCOLFRAMESIZE, "Content-length: %ld\r\n", pl->size);
  ClSize = pal_strlen(buffer);

  if( ClSize + pl->size > PROTOCOLFRAMESIZE )
  {
    Display(LOG_LEVEL_1, ELError, "tspSend", GOGO_STR_PAYLOAD_BIGGER_PROTOFRMSIZE);
    return -1;
  }

  memcpy(buffer + ClSize,pl->payload, pl->size);
  buffer[ClSize + pl->size] = 0;

  Display(LOG_LEVEL_3, ELInfo, "tspSend", STR_NET_SENDING, buffer);
  ret = nt->netsend(socket, buffer, ClSize + pl->size);
  if( ret < 0 )
  {
    Display(LOG_LEVEL_1, ELError, "tspSend", STR_NET_FAIL_W_SOCKET);
    return ret;
  }

  return ret;
}

/*

  ** COMMENTED OUT ON March 11, 2008. Code not used.
  ** Will need to be validated.

// --------------------------------------------------------------------------
// tspReceive:
//
sint32_t tspReceive(pal_socket_t socket, tPayload *pl, net_tools_t *nt)
{
  sint32_t BytesTotal = 0, BytesRead = 0, BytesLeft = 0;
  char Buffer[PROTOCOLFRAMESIZE+1];
  char *StartOfPayload;

  memset(Buffer,0,sizeof(Buffer));

  if (pl->payload) pal_free(pl->payload);
  if ((BytesRead = nt->netrecv(socket, Buffer, sizeof(Buffer))) <= 0)
  {
    Display(LOG_LEVEL_1, ELError, "tspReceive", STR_NET_FAIL_R_SOCKET);
    return -1;
  }

  Display(LOG_LEVEL_3, ELInfo, "tspReceive", STR_NET_RECEIVED, Buffer);

  if( memcmp(Buffer, "Content-length:", 15) )
  {
    Display(LOG_LEVEL_1, ELError, "tspReceive", GOGO_STR_EXPECTED_CONTENT_LENGTH, Buffer);
    return -1;
  }

  // Start of payload is after Content-length: XX\r\n

  if( (StartOfPayload = strchr(Buffer,'\n')) == NULL )
  {
    Display(LOG_LEVEL_1, ELError, "tspReceive", GOGO_STR_INVALID_RESPONSE_RECEIVED);
    return -1;
  }

  StartOfPayload++;
  BytesTotal = pal_strlen(StartOfPayload);

  if (((pl->size = atol(Buffer + 15)) <= 0L) || BytesTotal > pl->size)
  {
    Display(LOG_LEVEL_1, ELError, "tspReceive", GOGO_STR_INVALID_PAYLOAD_SIZE);
    return -1;
  }

  BytesLeft = pl->size - BytesTotal;
  while( BytesLeft > 0 )
  {
    if((BytesRead = nt->netrecv(socket, (StartOfPayload + BytesTotal), BytesLeft)) <= 0)
    {
      Display(LOG_LEVEL_1, ELError, "tspReceive", STR_NET_FAIL_R_SOCKET);
      return -1;
    }

    BytesTotal += BytesRead;
    BytesLeft -= BytesRead;
  }

  if((pl->payload = (char *)pal_malloc((pl->size) + 1)) == NULL) {
    Display(LOG_LEVEL_1, ELError, "tspReceive", STR_GEN_MALLOC_ERROR);
    return(PROTOCOL_EMEM);
  }

  memset(pl->payload, 0, sizeof(pl->payload));
  pal_strcpy(pl->payload, StartOfPayload);

  return PROTOCOL_OK;
}
*/
