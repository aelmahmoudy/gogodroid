/*
-----------------------------------------------------------------------------
 $Id: net.c,v 1.1 2009/11/20 16:53:38 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"
#include "gogoc_status.h"

#include "tsp_net.h"
#include "net.h"
#include "log.h"
#include "hex_strings.h"


// --------------------------------------------------------------------------
// parse_addr_port:
//
// This function will separate the hostname(or address) and port from a
// string that contains both in the form:
//   inet4: "hostname.example.org:3756"
//   inet4: "24.58.29.78:3756"
//   inet6: "hostnamev6.example.com:3563"
//   inet6: "[2001:5c0::BABA]:3563"
// If port is not specified, the default port number will be used.
//
// NOTE: MEMORY IS ALLOCATED HERE, USE free TO FREE 'addr' MEMORY.
//
// Return value: 0 on success, any other value means error.
//
sint32_t parse_addr_port( const char* addr_port, char** addr, uint16_t* port, uint16_t dflt_port )
{
  char buffer[MAXNAME];     // Memory buffer for string manipulations.
  char *srvname=NULL;
  char *srvport=NULL;


  // Parameter validation.
  if( addr == NULL ) return -1;

  // Work on a copy of 'addr_port'.
  pal_snprintf( buffer, sizeof buffer, "%s", addr_port);


  // -------------------
  // Parse server name.
  // -------------------
  srvname = strchr( buffer,'[' );
  if( srvname != NULL )
  {
    // IPv6 address.
    srvname = strtok( buffer, "]" );
    srvname = buffer+1;
  }
  else
  {
    // IPv4 address
    srvname = strtok( buffer, ":" );
  }

  // Alloc memory and copy server name in 'addr'.
  *addr = (char*) pal_malloc( pal_strlen(srvname) + 1 );
  if( !*addr )
  {
    Display(LOG_LEVEL_1, ELError, "parse_addr_port", STR_GEN_MALLOC_ERROR );
    return -1;
  }
  pal_strcpy( *addr, srvname );


  // -------------------
  // Parse port number.
  // -------------------
  srvport = strtok( NULL, ":" );
  if( srvport != NULL )
  {
    sint32_t si32_port = atoi(srvport);

    // Port was specified, perform integer validation.
    if( si32_port <= 0 || si32_port >= 65536 )
    {
      Display( LOG_LEVEL_1, ELError, "parse_addr_port", GOGO_STR_SERVICE_PORT_INVALID, srvport );
      pal_free( *addr );
      return -1;
    }
    *port = (uint16_t)si32_port;
  }
  else
  {
    // Port was not specified, use default port.
    *port = dflt_port;
  }

  return 0;
}


/*
 * Convert IPv4 address from string format to in_addr structure.
 * Port information are discarted.
 *
 * in_addr structure must be supplied and allocated
 *
 * return NULL if errors, supplied in_addr structure if success.
 */
struct in_addr *NetText2Addr(char *Address, struct in_addr *in_p)
{
  struct addrinfo hints;
  struct addrinfo *res=NULL, *result=NULL;
  char addr_cp[MAXSERVER];
  char *addr;

  if (NULL == Address || NULL == in_p)
    return NULL;

  /* Prepare hints structure */
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = PF_UNSPEC;

  /* copy the string before using strtok */
  strcpy(addr_cp, Address);

  /* be sure it is not an in-brackets v6 address */
  if ((strchr(Address, '[') != NULL) ||
      (strchr(Address, ']') != NULL))
    goto error_v4;

  // be sure no more than on ':' is used to specify port number
  // (not IPv6 address without brakets!
  if( (addr = strchr(Address, ':')) != NULL )
  {
    if( strchr(addr+1,':') != NULL )
      goto error_v4;
  }

  /* Remove port number if any */
  addr = addr_cp;
  strtok(addr_cp, ":");

  if( (getaddrinfo(addr, NULL, &hints, &res)) == 0 )
  {
    for(result = res; result; result = result->ai_next)
    {
      if (result->ai_family != AF_INET)
        continue;
      memcpy(in_p,
       &((struct sockaddr_in *)result->ai_addr)->sin_addr,
       sizeof(struct in_addr));
      freeaddrinfo(res);
      return in_p;
    }
  }

 error_v4:
  /* Cannot resolve */
  Display(LOG_LEVEL_3, ELWarning, "NetText2Addr", GOGO_STR_SERVER_NOT_IPV4);

  if (res != NULL)
    freeaddrinfo(res);

  return NULL;
}

/*
 * Convert IPv6 address from string format to in6_addr structure.
 *
 * The following format are supported (port information is ignored):
 *
 * "X:X::X:X"
 * "[X:X::X:X]"
 * "[X:X::X:X]:port"
 * "hostname"
 * "hostname:port"
 *
 * in6_addr structure must be supplied and allocated
 *
 * return NULL if errors, supplied in6_addr structure if success.
 */
struct in6_addr *NetText2Addr6(char *Address, struct in6_addr *in6_p)
{
  struct addrinfo hints;
  struct addrinfo *res    = NULL;
  struct addrinfo *result = NULL;
  char addr[MAXSERVER];
  char *p;
  int c = 0;

  if (NULL == Address || NULL == in6_p)
    return NULL;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = PF_INET6;

  /* Copy the address before stripping */
  addr[sizeof(addr) - 1] = '\0';
  strncpy(addr, Address, sizeof(addr) - 1);

  /*
   * Numeric address has more than one colon
   */
  for(p = addr; *p != '\0'; p++)
    if (':' == *p)
      c++;

  p = addr;

  if (c > 1) {
    /* Numeric address */
    hints.ai_flags = AI_NUMERICHOST;
    /* Strip the bracket and port information if any */
    if ('[' == *p) {
      strtok(p, "]");
      p++; /* Skip [ */
    }
  } else {
    /* Hostname: strip the port information if any */
    strtok(p, ":");
  }

  if ((getaddrinfo(p, NULL, &hints, &res)) == 0) {

    for (result = res; result; result = result->ai_next) {
      if (result->ai_family != AF_INET6)
        continue;
      memcpy(in6_p,
       &((struct sockaddr_in6 *)result->ai_addr)->sin6_addr,
       sizeof(struct in6_addr));
      freeaddrinfo(res);
      return in6_p;
    }

  }

  /* Cannot resolve */
  Display(LOG_LEVEL_3, ELWarning, "NetText2Addr6", GOGO_STR_SERVER_NOT_IPV6);

  if (res != NULL)
    freeaddrinfo(res);

  return NULL;

}
