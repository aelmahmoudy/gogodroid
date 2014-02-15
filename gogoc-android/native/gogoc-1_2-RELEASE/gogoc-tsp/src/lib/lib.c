/*
-----------------------------------------------------------------------------
 $Id: lib.c,v 1.1 2009/11/20 16:53:37 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"
#include "hex_strings.h"

#include "lib.h"

// these must be kept in sync with errors.h

char *TspErrorCodesArray[] = {
  "NO_ERROR",
  "NO_ERROR_SHOW_HELP",
  "TSP_ERROR",
  "SOCKET_ERROR",
  "INTERFACE_SETUP_FAILED",
  "KEEPALIVE_TIMEOUT",
  "KEEPALIVE_ERROR",
  "TUNNEL_ERROR",
  "TSP_VERSION_ERROR",
  "AUTHENTICATION_ERROR",
  "LEASE_EXPIRED",
  "SERVER_SIDE_ERROR",
  "INVALID_ARGUMENTS",
  "MEMORY_ERROR",
  "INVALID_SERVER",
  "INVALID_CONFIG_FILE",
  "INVALID_CLIENT_IPV4",
  "INVALID_CLIENT_IPV6",
  "LOGGING_CONFIGURATION_ERROR",
  "BROKER_REDIRECTION",
  "BROKER_REDIRECTION_ERROR",
  "SOCKET_ERROR_CANT_CONNECT",
  "INITIALIZATION_ERROR",
#ifdef HACCESS
  "HACCESS_INITIALIZATION_ERROR",
  "HACCESS_SETUP_ERROR",
  "HACCESS_EXPOSE_DEVICES_ERROR",
#endif
  NULL
};

// Prototype.
sint32_t GetSizeOfNullTerminatedArray( char ** );


/*
   Check if all characters in Value are within AllowedChars list.
*/
sint32_t IsAll( char *AllowedChars, char *Value )
{
  if( Value )
  {
    for(;*Value; Value++)
    {
      if(strchr(AllowedChars, *Value) == NULL)
        return 0;
    }
  }
  else
  {
    return 0;
  }

  return 1;
}

/*
   Check to see if there is a value in the char *
   If not, then the value was not supplied.
*/
sint32_t IsPresent( char *Value )
{
  if( Value && pal_strlen(Value) )
    return 1;
  return 0;
}

/* This next function is very dangerous.
   If you can call be certain the array
   finished by NULL or it will do bad things.
*/
sint32_t GetSizeOfNullTerminatedArray(char **a)
{
  sint32_t i;
  for (i = 0;;i++) 
  {
    if (a[i] == NULL)
      return i;
  }
  // unreachable code.
}


char *tspGetErrorByCode(sint32_t code)
{
  static char buf[1024];
  sint32_t i;

  i = GetSizeOfNullTerminatedArray(TspErrorCodesArray);
  if (code < i && code > -1)
    return TspErrorCodesArray[code];
  else
    pal_snprintf(buf, sizeof(buf), GOGO_STR_NOT_DEF_AS_CLIENT_ERROR, code);
  return buf;
}


/* Function: IsAddressInPrefix

  Verifies if a given IPv6 address is part of the given IPv6 prefix.

  Returns 0 if address is part of prefix.
  Returns 1 if address is NOT part of prefix.
  Returns -1 if either address or prefix is an invalid IPv6 address.

*/
sint32_t IsAddressInPrefix( const char* address, const char* prefix, const sint16_t prefix_len )
{
  short compare_bytes=0;
  short compare_bits =0;
  sint32_t ret_code=1;
  struct addrinfo hints, *res_address=NULL, *res_prefix=NULL;
  struct in6_addr *in6_address, *in6_prefix;

  memset( &hints, 0x00, sizeof(struct addrinfo) );
  hints.ai_family = AF_INET6;

  while( 1 )
  {
    if( (prefix_len > 0   &&  prefix_len <= 128) &&
        (address != NULL  &&  prefix != NULL   ) &&
        (getaddrinfo( address, NULL, &hints, &res_address ) != 0  ||  getaddrinfo( prefix, NULL, &hints, &res_prefix )   != 0) )
    {
      ret_code = -1;
      break;
    }

    if( res_address == NULL  ||  res_prefix == NULL )
    {
      ret_code = -1;
      break;
    }
    in6_address   = &((struct sockaddr_in6*)(res_address->ai_addr))->sin6_addr;
    in6_prefix    = &((struct sockaddr_in6*)(res_prefix->ai_addr))->sin6_addr;

    /* Compute how many bytes and bits to compare */
    compare_bytes = prefix_len / 8;
    compare_bits  = prefix_len % 8;

    /* Compare the bytes of address and prefix */
    if( memcmp( in6_address, in6_prefix, compare_bytes ) != 0 )
      break;

    /* Compare the bits */
    if( compare_bits > 0 )
      if( (in6_address->s6_addr[compare_bytes] >> compare_bits) != (in6_prefix->s6_addr[compare_bytes] >> compare_bits) )
        break;

    /* address is part of prefix */
    ret_code = 0;
    break;
  }

  /* Free memory used. */
  if( res_address ) freeaddrinfo( res_address );
  if( res_prefix  ) freeaddrinfo( res_prefix );


  return ret_code;
}


