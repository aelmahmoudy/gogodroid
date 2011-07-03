/*
-----------------------------------------------------------------------------
 $Id: tsp_auth.c,v 1.1 2009/11/20 16:53:40 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"
#include "gogoc_status.h"

#include "tsp_redirect.h"
#include "tsp_client.h"
#include "tsp_auth.h"

#ifndef NO_OPENSSL
#include "tsp_auth_passdss.h"
#endif

#include "log.h"
#include "hex_strings.h"
#include "base64.h"
#include "md5.h"
#include "version.h"


// Challenge structure.
typedef struct  {
  char *realm,
      *nonce,
      *qop,
      *algorithm,
      *charset,
      *rspauth;
} tChallenge;


// --------------------------------------------------------------------------
// AuthANONYMOUS: Performs an anonymous authentication with the server.
//
gogoc_status AuthANONYMOUS(pal_socket_t socket, net_tools_t *nt, tConf *conf, tBrokerList **broker_list)
{
  char Buffer[REDIRECT_RECEIVE_BUFFER_SIZE];
  char string[] = "AUTHENTICATE ANONYMOUS\r\n";
  sint32_t tsp_status;


  // Send authentication mode.
  if( nt->netsendrecv(socket, string, sizeof(string), Buffer, sizeof(Buffer)) == -1 )
  {
    Display(LOG_LEVEL_1, ELError, "AuthANONYMOUS", STR_NET_FAIL_RW_SOCKET);
    return make_status(CTX_TSPAUTHENTICATION, ERR_SOCKET_IO);
  }
  tsp_status = tspGetStatusCode(Buffer);

  // Check if the reply status indicated a broker redirection.
  if( tspIsRedirectStatus(tsp_status) )
  {
    if( tspHandleRedirect(Buffer, conf, broker_list) == TSP_REDIRECT_OK )
    {
      // Return a REDIRECT event.
      return make_status(CTX_TSPAUTHENTICATION, EVNT_BROKER_REDIRECTION);
    }
    else 
    {
      // Redirect error.
      return make_status(CTX_TSPAUTHENTICATION, ERR_BROKER_REDIRECTION);
    }
  }

  // Check if authentication was successful.
  // No need to handle TSP_PROTOCOL_AUTH_FAILED here...
  if( tsp_status != TSP_PROTOCOL_SUCCESS )
  {
    Display(LOG_LEVEL_1, ELError, "AuthANONYMOUS", STR_TSP_UNKNOWN_ERR_AUTH_FAILED, tspGetTspStatusStr(tsp_status));
    return make_status(CTX_TSPAUTHENTICATION, ERR_TSP_GENERIC_ERROR);
  }

  // Successful anonymous authentication.
  return make_status(CTX_TSPAUTHENTICATION, SUCCESS);
}


// --------------------------------------------------------------------------
// AuthPLAIN: Performs a plain-text username and password authentication with
//   the server.
//
gogoc_status AuthPLAIN(pal_socket_t socket, net_tools_t *nt, tConf *conf, tBrokerList **broker_list)
{
  char BufferIn[1024];
  char BufferOut[REDIRECT_RECEIVE_BUFFER_SIZE];
  char string[] = "AUTHENTICATE PLAIN\r\n";
  int Length;
  sint32_t tsp_status;


  // Send authentication mode.
  if( nt->netsend(socket, string, sizeof(string)) == -1 )
  {
    Display(LOG_LEVEL_1, ELError, "AuthPLAIN", STR_NET_FAIL_W_SOCKET);
    return make_status(CTX_TSPAUTHENTICATION, ERR_SOCKET_IO);
  }

  memset(BufferIn, 0, sizeof(BufferIn));
  Length = pal_snprintf(BufferIn, sizeof(BufferIn), "%c%s%c%s\r\n", '\0', conf->userid, '\0', conf->passwd);

  // Send username/password for authentication.
  if( nt->netsendrecv(socket, BufferIn, Length, BufferOut, sizeof(BufferOut)) == -1 )
  {
    Display(LOG_LEVEL_1, ELError, "AuthPLAIN", STR_NET_FAIL_RW_SOCKET);
    return make_status(CTX_TSPAUTHENTICATION, ERR_SOCKET_IO);
  }
  tsp_status = tspGetStatusCode(BufferOut);

  // Check if the reply status indicated a broker redirection.
  if( tspIsRedirectStatus(tsp_status) )
  {
    if( tspHandleRedirect(BufferOut, conf, broker_list) == TSP_REDIRECT_OK )
    {
      // Return a REDIRECT event.
      return make_status(CTX_TSPAUTHENTICATION, EVNT_BROKER_REDIRECTION);
    }
    else 
    {
      // Redirect error.
      return make_status(CTX_TSPAUTHENTICATION, ERR_BROKER_REDIRECTION);
    }
  }

  // Check if authentication was successful.
  switch( tsp_status )
  {
  case TSP_PROTOCOL_SUCCESS:
    break;

  case TSP_PROTOCOL_AUTH_FAILED:
    Display(LOG_LEVEL_1, ELError, "AuthPLAIN", STR_TSP_AUTH_FAILED_USER, conf->userid);
    return make_status(CTX_TSPAUTHENTICATION, ERR_AUTHENTICATION_FAILURE);

  default:
    Display(LOG_LEVEL_1, ELError, "AuthPLAIN", STR_TSP_UNKNOWN_ERR_AUTH_FAILED, tspGetTspStatusStr(tsp_status));
    return make_status(CTX_TSPAUTHENTICATION, ERR_TSP_GENERIC_ERROR);
  }

  // Successful plain text authentication.
  return make_status(CTX_TSPAUTHENTICATION, SUCCESS);
}


// --------------------------------------------------------------------------
// InsertInChallegeStruct: Helper function to fill in a tChallenge struct.
//
sint32_t InsertInChallegeStruct(tChallenge *c, char *Token, char *Value)
{

  if(strcmp(Token, "realm")==0) {
    c->realm = pal_strdup(Value);
    return 0;
  }

  if(strcmp(Token, "nonce")==0) {
    c->nonce = pal_strdup(Value);
    return 0;
  }

  if(strcmp(Token, "qop")==0) {
    c->qop = pal_strdup(Value);
    return 0;
  }

  if(strcmp(Token, "algorithm")==0) {
    c->algorithm = pal_strdup(Value);
    return 0;
  }

  if(strcmp(Token, "charset")==0) {
    c->charset = pal_strdup(Value);
    return 0;
  }

  if(strcmp(Token, "rspauth")==0) {
    c->rspauth = pal_strdup(Value);
    return 0;
  }

  return -1;
}


// --------------------------------------------------------------------------
//
//
void ExtractChallenge(tChallenge *c, char *String)
{
  char *s, *e, *Token, *Value;
  int len;

  memset(c, 0, sizeof(tChallenge));

  Token = (char*) pal_malloc( pal_strlen(String)+1 );
  Value = (char*) pal_malloc( pal_strlen(String)+1 );

  *Token=*Value=0;

  for(s=e=String; ; e++) {
    if(*e== ',' || *e == '\r' || *e == '\n' || *e==0) {
      if(s!=e) {
        if(*Token && (*Value==0)) {
          len = (int)((char *)e-(char *)s);
/* Chop the quotes */
          if((*s == '"') && len) { s++; len--; }
          if((s[len-1] == '"') && len) len--;
          if(len) memcpy(Value, s, len);
          Value[len] = 0;
        }
        if(*Token && *Value) {
          InsertInChallegeStruct(c, Token,Value);
          *Value = *Token = 0;
        }
      }

      if(*e == 0) break;
      s = ++e;
    }

    if((*e=='=' || *e== ',' || *e == '\r' || *e == '\n' || *e==0) && (*Token == 0) && (e != s)) {
      len = (int)((char *)e-(char *)s);
      memcpy(Token, s, len);
      Token[len] = 0;
      if(*e == 0) break;
      s = ++e;
    }
  }

  pal_free( Token );
  pal_free( Value );
}


// --------------------------------------------------------------------------
// AuthDIGEST_MD5: Performs a Digest-MD5 authentication with the server.
//
gogoc_status AuthDIGEST_MD5(pal_socket_t socket, net_tools_t *nt, tConf *conf, tBrokerList **broker_list, int version_index)
{
  char Buffer[4096], Response[33], cResponse[33], *ChallengeString;
  char string[] = "AUTHENTICATE DIGEST-MD5\r\n";
  char BufferIn[REDIRECT_RECEIVE_BUFFER_SIZE];
  time_t cnonce = pal_time(NULL);
  tChallenge c;
  sint32_t tsp_status;


  // Send authentication mode.
  memset(BufferIn, 0, sizeof(BufferIn));
  if( nt->netsendrecv(socket, string, sizeof(string), BufferIn, sizeof(BufferIn)) == -1 )
  {
    Display(LOG_LEVEL_1, ELError, "AuthDIGEST_MD5", STR_NET_FAIL_RW_SOCKET);
    return make_status(CTX_TSPAUTHENTICATION, ERR_SOCKET_IO);
  }
  tsp_status = tspGetStatusCode(BufferIn);

  // Check if the reply status indicated a broker redirection.
  if( tspIsRedirectStatus(tsp_status) )
  {
    if( tspHandleRedirect(BufferIn, conf, broker_list) == TSP_REDIRECT_OK )
    {
      // Return a REDIRECT event.
      return make_status(CTX_TSPAUTHENTICATION, EVNT_BROKER_REDIRECTION);
    }
    else 
    {
      // Redirect error.
      return make_status(CTX_TSPAUTHENTICATION, ERR_BROKER_REDIRECTION);
    }
  }

  // Check for error in status.
  if( tsp_status == TSP_PROTOCOL_AUTH_FAILED )
  {
    // Failed authentication.
    Display(LOG_LEVEL_1, ELError, "AuthDIGEST_MD5", STR_TSP_AUTH_FAILED_USER, conf->userid);
    return make_status(CTX_TSPAUTHENTICATION, ERR_AUTHENTICATION_FAILURE);
  }

  // Allocate memory for challenge string.
  if( (ChallengeString = pal_malloc(pal_strlen(BufferIn) + 1)) == NULL )
  {
    Display(LOG_LEVEL_1, ELError, "AuthDIGEST_MD5", STR_GEN_MALLOC_ERROR);
    return make_status(CTX_TSPAUTHENTICATION, ERR_MEMORY_STARVATION);
  }

  base64decode(ChallengeString, BufferIn);
  ExtractChallenge(&c, ChallengeString);
  pal_free(ChallengeString);

   {
   /*-----------------------------------------------------------*/
   /*
      Extract from : RFC 2831 Digest SASL Mechanism

      Let H(s) be the 16 octet MD5 hash [RFC 1321] of the octet string s.

      Let KD(k, s) be H({k, ":", s}), i.e., the 16 octet hash of the string
      k, a colon and the string s.

      Let HEX(n) be the representation of the 16 octet MD5 hash n as a
      string of 32 hex digits (with alphabetic characters always in lower
      case, since MD5 is case sensitive).

      response-value  =
         HEX( KD ( HEX(H(A1)),
                 { nonce-value, ":" nc-value, ":",
                   cnonce-value, ":", qop-value, ":", HEX(H(A2)) }))

      If authzid is not specified, then A1 is

         A1 = { H( { username-value, ":", realm-value, ":", passwd } ),
           ":", nonce-value, ":", cnonce-value }

      If the "qop" directive's value is "auth", then A2 is:

         A2       = { "AUTHENTICATE:", digest-uri-value }

   */
    char *A1_1Fmt     = "%s:%s:%s",
#ifndef WIN32
         *A1Fmt        = ":%s:%lu",
         *ChallRespFmt = "%s:%s:00000001:%lu:%s:%s",
         *ResponseFmt  = "charset=%s,username=\"%s\",realm=\"%s\",nonce=\"%s\",nc=00000001,cnonce=\"%lu\",digest-uri=\"tsp/%s\",response=%s,qop=auth",
#else
          // 64 bit version.
         *A1Fmt        = ":%s:%I64d",
         *ChallRespFmt = "%s:%s:00000001:%I64d:%s:%s",
         *ResponseFmt  = "charset=%s,username=\"%s\",realm=\"%s\",nonce=\"%s\",nc=00000001,cnonce=\"%I64d\",digest-uri=\"tsp/%s\",response=%s,qop=auth",
#endif
         *A2Fmt        = "%s:tsp/%s",
         A1[33], A1_1[33], A2[33], cA2[33], *String;
    size_t len;

    /*-----------------------------------------------------------*/
    /* Build HEX(H(A2)) & HEX(H(cA2))                            */

    len = pal_strlen(A2Fmt) + 12 /* AUTHENTICATE */ + pal_strlen(conf->server) + 1;
    if( (String = pal_malloc(len)) == NULL ) 
    {
      Display(LOG_LEVEL_1, ELError, "AuthDIGEST_MD5", STR_GEN_MALLOC_ERROR);
      return make_status(CTX_TSPAUTHENTICATION, ERR_MEMORY_STARVATION);
    }

     pal_snprintf(String, len, A2Fmt, "AUTHENTICATE", conf->server);
#if defined(_DEBUG) || defined(DEBUG)
     printf("A2 = %s\n", String);
#endif
     strncpy(A2, md5(String, pal_strlen(String)), 33);
     pal_snprintf(String, len, A2Fmt, "", conf->server);
#if defined(_DEBUG) || defined(DEBUG)
     printf("cA2 = %s\n", String);
#endif
     strncpy(cA2, md5(String, pal_strlen(String)), 33);
     pal_free(String);

    /*-----------------------------------------------------------*/
    /* Build HEX(H(A1))                                          */
    /* A1_1 = { username-value, ":", realm-value, ":", passwd }  */
    /* A1 = { H( A1_1 ), ":", nonce-value, ":", cnonce-value }   */

    len = pal_strlen(A1_1Fmt) + pal_strlen(conf->userid) + 
          pal_strlen(c.realm) + pal_strlen(conf->passwd) +  1;
    if( (String = pal_malloc(len)) == NULL )
    {
      Display(LOG_LEVEL_1, ELError, "AuthDIGEST_MD5", STR_GEN_MALLOC_ERROR);
      return make_status(CTX_TSPAUTHENTICATION, ERR_MEMORY_STARVATION);
    }

     pal_snprintf(String, len, A1_1Fmt, conf->userid, c.realm, conf->passwd);
#if defined(_DEBUG) || defined(DEBUG)
     printf("A1_1 = %s\n", String);
#endif
     md5digest(String, pal_strlen(String), A1_1);
     pal_free(String);
     len = 16 /* A1_1 */ + 1 +
         pal_strlen(c.nonce) + 16 /* cnonce */ +  1;
    if( (String = pal_malloc(len)) == NULL )
    {
      Display(LOG_LEVEL_1, ELError, "AuthDIGEST_MD5", STR_GEN_MALLOC_ERROR);
      return make_status(CTX_TSPAUTHENTICATION, ERR_MEMORY_STARVATION);
    }

     memcpy(String, A1_1, 16);
     pal_snprintf(String + 16, len - 16, A1Fmt, c.nonce, cnonce);
#ifdef SUPPORT_MD5_BUG1455
     A1_1[16] = '\0';
     if ((pal_strlen(A1_1) < 16) &&
        !((pal_strlen(TSPProtoVerStr[version_index]) > 5) ||
    (strcmp(TSPProtoVerStr[version_index], CLIENT_VERSION_STRING_2_0_0) > 0)))
        strncpy(A1, md5(String, pal_strlen(String)), 33);
     else
#endif /* SUPPORT_MD5_BUG1455 */
         strncpy(A1, md5(String, 16 + pal_strlen(String + 16)), 33);
     pal_free(String);
#if defined(_DEBUG) || defined(DEBUG)
     printf("A1 = [%s]\n", A1);
#endif

    /*-----------------------------------------------------------*/
    /* Build server's and client's challenge responses           */
    len = pal_strlen(ChallRespFmt) + 32 /* md5(A1) */ + pal_strlen(c.nonce) +16 /* cnonce */ + pal_strlen(c.qop) + 32 /* md5(A2) */ +  1;
    if((String = pal_malloc(len)) == NULL)
    {
      Display(LOG_LEVEL_1, ELError, "AuthDIGEST_MD5", STR_GEN_MALLOC_ERROR);
      return make_status(CTX_TSPAUTHENTICATION, ERR_MEMORY_STARVATION);
    }

     pal_snprintf(String, len, ChallRespFmt, A1, c.nonce, cnonce, c.qop, A2);
#if defined(_DEBUG) || defined(DEBUG)
     printf("Response = [%s]\n", String);
#endif
     strncpy(Response, md5(String, pal_strlen(String)), 33);
#if defined(_DEBUG) || defined(DEBUG)
     printf("MD5 Response = %s\n", Response);
#endif
     pal_snprintf(String, len, ChallRespFmt, A1, c.nonce, cnonce, c.qop, cA2);
#if defined(_DEBUG) || defined(DEBUG)
     printf("cResponse = [%s]\n", String);
#endif
     strncpy(cResponse, md5(String, pal_strlen(String)), 33);
#if defined(_DEBUG) || defined(DEBUG)
     printf("MD5 cResponse = %s\n", cResponse);
#endif
     pal_free(String);

      /*-----------------------------------------------------------*/
      /* Build Response                                            */
     {
       char   userid[512];  // UserId is theorically limited to 253 chars.
       char * cc;
       size_t i;

        // Escape malicious " and \ from conf->userid.
       for(cc=conf->userid, i=0; *cc && i<512; cc++, i++)
       {
         // Prepend a backslash (\).
         if( *cc == '"'  ||  *cc == '\\' )
           userid[i++] = '\\';
          // Copy character.
          userid[i] = *cc;
       }
       userid[i] = '\0';

       len = pal_strlen(ResponseFmt) + pal_strlen(c.charset) + pal_strlen(userid) +
           pal_strlen(c.realm) + pal_strlen(c.nonce) + 16 /*cnonce*/ +
           pal_strlen(conf->server)    + 32 /* md5 response */;
       if( (String = pal_malloc(len)) == NULL )
       {
        Display(LOG_LEVEL_1, ELError, "AuthDIGEST_MD5", STR_GEN_MALLOC_ERROR);
        return make_status(CTX_TSPAUTHENTICATION, ERR_MEMORY_STARVATION);
       }

       pal_snprintf(String, len, ResponseFmt, c.charset, userid, c.realm, c.nonce, cnonce, conf->server, Response);
       memset(Buffer, 0, sizeof(Buffer));
       base64encode(Buffer, String, (int)pal_strlen(String));
       pal_free(String);
     }
   }

  // Send authentication data.
  memset(BufferIn, 0, sizeof(BufferIn));
  if( nt->netprintf(socket, BufferIn, sizeof(BufferIn), "%s\r\n", Buffer) == -1 )
  {
    Display(LOG_LEVEL_1, ELError, "AuthDIGEST_MD5", STR_NET_FAIL_W_SOCKET);
    return make_status(CTX_TSPAUTHENTICATION, ERR_SOCKET_IO);
  }
  tsp_status = tspGetStatusCode(BufferIn);

  // Check if the reply status indicated a broker redirection.
  if( tspIsRedirectStatus(tsp_status) )
  {
    if( tspHandleRedirect(BufferIn, conf, broker_list) == TSP_REDIRECT_OK )
    {
      // Return a REDIRECT event.
      return make_status(CTX_TSPAUTHENTICATION, EVNT_BROKER_REDIRECTION);
    }
    else 
    {
      // Redirect error.
      return make_status(CTX_TSPAUTHENTICATION, ERR_BROKER_REDIRECTION);
    }
  }

  /*-----------------------------------------------------------*/
  /* Verify server response                                    */
  if( tsp_status == TSP_PROTOCOL_AUTH_FAILED )
  {
    // Failed authentication.
    Display(LOG_LEVEL_1, ELError, "AuthDIGEST_MD5", STR_TSP_AUTH_FAILED_USER, conf->userid);
    return make_status(CTX_TSPAUTHENTICATION, ERR_AUTHENTICATION_FAILURE);
  }

  if( (ChallengeString = pal_malloc(pal_strlen(BufferIn) + 1)) == NULL )
  {
    Display(LOG_LEVEL_1, ELError, "AuthDIGEST_MD5", STR_GEN_MALLOC_ERROR);
    return make_status(CTX_TSPAUTHENTICATION, ERR_MEMORY_STARVATION);
  }

  base64decode(ChallengeString, BufferIn);
  ExtractChallenge(&c, ChallengeString);
  pal_free(ChallengeString);

  if( memcmp(c.rspauth, cResponse, 32) )
  {
    Display(LOG_LEVEL_1, ELError, "AuthDIGEST_MD5", STR_MISC_INVALID_MD5_RESPONSE);
    return make_status(CTX_TSPAUTHENTICATION, ERR_AUTHENTICATION_FAILURE);
  }

  // Receive reply.
  if( nt->netrecv(socket, Buffer, sizeof(Buffer) ) == -1 )
  {
    Display(LOG_LEVEL_1, ELError, "AuthDIGEST_MD5", STR_NET_FAIL_R_SOCKET);
    return make_status(CTX_TSPAUTHENTICATION, ERR_SOCKET_IO);
  }
  tsp_status = tspGetStatusCode(Buffer);

  // Check if the reply status indicated a broker redirection.
  if( tspIsRedirectStatus(tsp_status) )
  {
    if( tspHandleRedirect(Buffer, conf, broker_list) == TSP_REDIRECT_OK )
    {
      // Return a REDIRECT event.
      return make_status(CTX_TSPAUTHENTICATION, EVNT_BROKER_REDIRECTION);
    }
    else 
    {
      // Redirect error.
      return make_status(CTX_TSPAUTHENTICATION, ERR_BROKER_REDIRECTION);
    }
  }

  // Check if authentication was successful.
  switch( tsp_status )
  {
  case TSP_PROTOCOL_SUCCESS:
    break;

  case TSP_PROTOCOL_AUTH_FAILED:
    Display(LOG_LEVEL_1, ELError, "AuthDIGEST_MD5", STR_TSP_AUTH_FAILED_USER, conf->userid);
    return make_status(CTX_TSPAUTHENTICATION, ERR_AUTHENTICATION_FAILURE);

  default:
    Display(LOG_LEVEL_1, ELError, "AuthDIGEST_MD5", STR_TSP_UNKNOWN_ERR_AUTH_FAILED, tspGetTspStatusStr(tsp_status));
    return make_status(CTX_TSPAUTHENTICATION, ERR_TSP_GENERIC_ERROR);
  }

  // Successful MD5 authentication.
  return make_status(CTX_TSPAUTHENTICATION, SUCCESS);
}


// --------------------------------------------------------------------------
// Function : tspAuthenticate
//
// Synopsys: Will authenticate a session with the broker.
//
// Description:
//   First, we'll try to find the most secure common authentication method.
//   Once the authentication method has been chosen, the authentication
//   process is initiated with the broker.
//
// Arguments: (only local-specific arguments are listed here)
//   cap: bitfield [IN], The authentication methods suported by the broker.
//   conf: tConf* [IN], The global configuration object.
//
// Return values:
//   A gogoc_status status.
//
// --------------------------------------------------------------------------
gogoc_status tspAuthenticate(pal_socket_t socket, tCapability cap, net_tools_t *nt, tConf *conf, tBrokerList **broker_list, int version_index)
{
  gogoc_status status = make_status(CTX_TSPAUTHENTICATION, ERR_NO_COMMON_AUTHENTICATION);
  tCapability Mechanism;


  // Get mechanism, depending on requested authentication method.
  if( pal_strcasecmp( conf->auth_method, "any" ) == 0 )
    Mechanism = AUTH_ANY;
  else
    Mechanism = tspSetCapability("AUTH", conf->auth_method);


  if( pal_strcasecmp( conf->auth_method, "anonymous" ) != 0 )
  {
    // Try the most secure authentication methods first:
#ifndef NO_OPENSSL
    if( Mechanism & cap & AUTH_PASSDSS_3DES_1 )
    {
      Display(LOG_LEVEL_3, ELInfo, "tspAuthenticate", GOGO_STR_USING_AUTH_PASSDSS_3DES_1);
      status = AuthPASSDSS_3DES_1(socket, nt, conf, broker_list);
      goto EndAuthenticate;
    }
#endif
    if( Mechanism & cap & AUTH_DIGEST_MD5 )
    {
      Display(LOG_LEVEL_3, ELInfo, "tspAuthenticate", GOGO_STR_USING_AUTH_DIGEST_MD5);
      status = AuthDIGEST_MD5(socket, nt, conf, broker_list, version_index);
      goto EndAuthenticate;
    }

    if( Mechanism & cap & AUTH_PLAIN )
    {
      Display(LOG_LEVEL_3, ELInfo, "tspAuthenticate", GOGO_STR_USING_AUTH_PLAIN);
      status = AuthPLAIN(socket, nt, conf, broker_list);
      goto EndAuthenticate;
    }
  }
  else
  {
    // Finally, try anonymous if possible.
    if( Mechanism & cap & AUTH_ANONYMOUS )
    {
      Display(LOG_LEVEL_3, ELInfo, "tspAuthenticate", GOGO_STR_USING_AUTH_ANONYMOUS);
      status = AuthANONYMOUS(socket, nt, conf, broker_list);
      goto EndAuthenticate;
    }
  }

EndAuthenticate:
  if( status_number(status) == ERR_NO_COMMON_AUTHENTICATION )
  {
    const char* szStrings[] = { "Server Authentication Capabilities: ",
                                "Your Configured Authentication:     " };
    size_t nWritten;
    char bufDisplay[256];

    // Display server authentication capabilities.
    pal_snprintf( bufDisplay, sizeof(bufDisplay), "%s", szStrings[0] );
    nWritten = pal_strlen( szStrings[0] );
    tspFormatCapabilities( bufDisplay + nWritten, sizeof(bufDisplay) - nWritten, cap );
    Display( LOG_LEVEL_1, ELWarning, "tspAuthenticate", bufDisplay );

    // Display user authentication choice.
    pal_snprintf( bufDisplay, sizeof(bufDisplay), "%s", szStrings[1] );
    nWritten = pal_strlen( szStrings[1] );
    tspFormatCapabilities( bufDisplay + nWritten, sizeof(bufDisplay) - nWritten, Mechanism );
    Display( LOG_LEVEL_1, ELWarning, "tspAuthenticate", bufDisplay );

    // Failed to find a common authentication method.
    Display(LOG_LEVEL_1, ELError, "tspAuthenticate", STR_TSP_NO_COMMON_AUTHENTICATION);
  }

  return status;
}
