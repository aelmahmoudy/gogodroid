/*
-----------------------------------------------------------------------------
 $Id: tsp_cap.c,v 1.1 2009/11/20 16:53:41 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"
#include "gogoc_status.h"

#include "tsp_cap.h"
#include "tsp_client.h"	// tspGetStatusCode()
#include "net.h"
#include "log.h"
#include "hex_strings.h"
#include "version.h"
#include "tsp_redirect.h"


// --------------------------------------------------------------------------
// Convert a CAPABILITY string in corresponding bit in capability flag.
//
tCapability tspExtractCapability(char *String)
{
	tCapability flags = 0;
	char *s, *e, *Token, *Value;
	int len;

  Token = (char*) malloc( strlen(String)+1 );
  Value = (char*) malloc( strlen(String)+1 );
	*Token = *Value = 0;

	for(s=e=String+11; *e; e++) {
		if(*e== ' ' || *e == '\r' || *e == '\n' || *e == 0) {
			if(s!=e) {
				if(*Token && (*Value == 0)) {
					len = (int)((char *)e-(char *)s);
					memcpy(Value, s, len);
					Value[len] = 0;
				}
				if(*Token && *Value) {
					flags |= tspSetCapability(Token,Value);
					*Value = *Token = 0;
				}
			}
			s = ++e;
		}

		if((*e=='=' || *e== ' ' || *e == '\r' || *e == '\n' || *e == 0) && (e != s)) {
			len = (int)((char *)e-(char *)s);
			memcpy(Token, s, len);
			Token[len] = 0;
			s = ++e;
		}
	}
  
  free( Token );
  free( Value );

	return flags;
}


// --------------------------------------------------------------------------
// Return the capability flag corrsponding to the token.
//
tCapability tspSetCapability(char *Token, char *Value)
{
	if(strcmp("TUNNEL", Token)==0) {
		if(strcmp("V6V4", Value)==0)
			return TUNNEL_V6V4;
		if(strcmp("V6UDPV4", Value)==0)
			return TUNNEL_V6UDPV4;
#ifdef V4V6_SUPPORT		
		if(strcmp("V4V6", Value)==0)
			return TUNNEL_V4V6;
#endif /* V4V6_SUPPORT */
	}
	if(strcmp("AUTH", Token)==0) {
#ifndef NO_OPENSSL
		if(pal_strcasecmp("PASSDSS-3DES-1", Value)==0)
			return AUTH_PASSDSS_3DES_1;
#endif
		if(pal_strcasecmp("DIGEST-MD5", Value)==0)
			return AUTH_DIGEST_MD5;
		if(pal_strcasecmp("ANONYMOUS", Value)==0)
			return AUTH_ANONYMOUS;
		if(pal_strcasecmp("PLAIN", Value)==0)
			return AUTH_PLAIN;
	}
	return 0;
}


// --------------------------------------------------------------------------
// tspGetCapabilities:
//
gogoc_status tspGetCapabilities(pal_socket_t socket, net_tools_t *nt, tCapability *capability, int version_index, tConf *conf, tBrokerList **broker_list)
{
	char dataout[256];
	char datain[REDIRECT_RECEIVE_BUFFER_SIZE];
  sint32_t tsp_status;
  gogoc_status status = make_status(CTX_TSPCAPABILITIES, SUCCESS);


	memset( datain, 0, sizeof(datain) );
	pal_snprintf(dataout, sizeof(dataout), "VERSION=%s\r\n", TSPProtoVerStr[version_index]);

  // Send TSP version to the server. Server should reply with the capabilities.
	if( nt->netsendrecv(socket, dataout, pal_strlen(dataout), datain, (sint32_t)sizeof(datain)) == -1 )
  {
    // Error reading/writing to the socket.
		return make_status(CTX_TSPCAPABILITIES, ERR_SOCKET_IO);
  }

  // Check if we received the TSP capabilities.
	if( memcmp("CAPABILITY ", datain, 11) == 0 )
  {
    // Extract the capabilities.
		*capability = tspExtractCapability(datain);
  }
	else
  {
    // Retrieve the TSP status from the reply.
    tsp_status = tspGetStatusCode(datain);

    // Check if it is a redirect TSP status.
	  if( tspIsRedirectStatus(tsp_status) )
    {
		  if( tspHandleRedirect(datain, conf, broker_list) == TSP_REDIRECT_OK )
      {
        // REDIRECT event.
        status = make_status(CTX_TSPCAPABILITIES, EVNT_BROKER_REDIRECTION);
		  }
		  else 
      {
        // Redirect error.
			  status = make_status(CTX_TSPCAPABILITIES, ERR_BROKER_REDIRECTION);
		  }
	  }
    else
    {
      switch( tsp_status )
      {
      case TSP_PROTOCOL_SERVER_TOO_BUSY:
        // Ze server iz too busy.
        Display(LOG_LEVEL_1, ELWarning, "tspGetCapabilities", STR_TSP_SERVER_TOO_BUSY);
			  status = make_status(CTX_TSPCAPABILITIES, ERR_TSP_SERVER_TOO_BUSY);
        break;

      case TSP_PROTOCOL_UNSUP_TSP_VER:
        // The status was an invalid TSP version.
        Display(LOG_LEVEL_1, ELWarning, "tspGetCapabilities", STR_TSP_INVALID_VERSION, TSPProtoVerStr[version_index]);
			  status = make_status(CTX_TSPCAPABILITIES, ERR_INVAL_TSP_VERSION);
        break;

      default:
        // Unknown / unexpected TSP error occurred.
		    Display(LOG_LEVEL_1, ELError, "tspGetCapabilities", STR_TSP_GEN_ERROR, tsp_status, tspGetTspStatusStr(tsp_status));
		    status = make_status(CTX_TSPCAPABILITIES, ERR_TSP_GENERIC_ERROR);
        break;
      }
      Display(LOG_LEVEL_1, ELError, "tspGetCapabilities", STR_TSP_GETCAPABILITIES_ERROR);
    }
	}

  // Successful operation.
	return status;
}


// --------------------------------------------------------------------------
// Formats a comma-separated string containing the capability(ies) in the buffer provided.
// Returns the buffer.
char* tspFormatCapabilities( char* szBuffer, const size_t bufLen, const tCapability cap )
{
  static const char* authTab[] = { "Any, ", 
#ifndef NO_OPENSSL
                                   "Passdss-3des-1, ",
#endif
                                   "Digest MD5, ",
                                   "Anonymous, ",
                                   "Plain, " };
  size_t nWritten = 0;

  /*
    AUTH_ANY is explicitly expanded in the following capabilities.
  */

#ifndef NO_OPENSSL
  if( (cap & AUTH_PASSDSS_3DES_1)  == AUTH_PASSDSS_3DES_1 )
  {
    strncpy( szBuffer + nWritten, authTab[1], bufLen - nWritten );
    nWritten += strlen( authTab[1] );
  }
#endif

  if( (cap & AUTH_DIGEST_MD5)  == AUTH_DIGEST_MD5 )
  {
    strncpy( szBuffer + nWritten, authTab[2], bufLen - nWritten );
    nWritten += strlen( authTab[2] );
  }
  if( (cap & AUTH_ANONYMOUS)  ==  AUTH_ANONYMOUS  &&  (cap & AUTH_ANY)  !=  AUTH_ANY )
  {
    // ANY does not include ANONYMOUS
    strncpy( szBuffer + nWritten, authTab[3], bufLen - nWritten );
    nWritten += strlen( authTab[3] );
  }
  if( (cap & AUTH_PLAIN)  ==  AUTH_PLAIN )
  {
    strncpy( szBuffer + nWritten, authTab[4], bufLen - nWritten );
    nWritten += strlen( authTab[4] );
  }

  if( nWritten > 2 )
    szBuffer[nWritten-2] = '\0';  // Remove the end comma.


  return szBuffer;
}
