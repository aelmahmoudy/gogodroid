/*
-----------------------------------------------------------------------------
 $Id: net_rudp6.c,v 1.1 2009/11/20 16:53:39 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"

#include "net_rudp.h"
#include "net_rudp6.h"
#include "net.h"
#include "log.h"

#ifdef V4V6_SUPPORT

// forward declarations (IPv6 version of internal_get_sai())

static struct sockaddr_in6 *internal_get_sai6(rttengine_stat_t *, char *, unsigned short);


/* Exported functions */

sint32_t NetRUDP6Init(void) 
{
	memset(&rttengine_stats, 0, sizeof(rttengine_stat_t));
	return rttengine_init(&rttengine_stats);
}

/* */

sint32_t NetRUDP6Destroy(void) 
{
	if ( rttengine_deinit(&rttengine_stats, NULL, NULL) == 0)
		return 1;
	return 0;
}

// --------------------------------------------------------------------------
// NetRUDP6Connect:
//
// Return values:
//    0: success
//   -1: Failed to resolve srvname.
//   -2: Socket error / Failed to connect (TCP only).
//
sint32_t NetRUDP6Connect(pal_socket_t *p_sock, char *Host, unsigned short Port) 
{
	pal_socket_t sfd;
	struct sockaddr_in6 *sai;

	if (rttengine_stats.initiated == 0)
		NetRUDP6Init();

	if( (sai = internal_get_sai6(&rttengine_stats, Host, Port)) == NULL)
  {
		return -1;
	}


	/* and get a socket */

	if( (sfd = pal_socket(PF_INET6, SOCK_DGRAM, 0)) == -1 )
  {
		return -2;
	}

	/* then connect it */

	if( (pal_connect(sfd,(struct sockaddr *) sai, sizeof(struct sockaddr_in6))) == -1 )
  {
    pal_closesocket( sfd );
		return -2;
	}
	
  *p_sock = sfd;
	return 0;
}

/* */

sint32_t NetRUDP6Close( pal_socket_t sock ) 
{
	pal_shutdown( sock, PAL_SOCK_SHTDN_BOTH );
	pal_closesocket( sock );
	return NetRUDP6Destroy();
}

/* */

sint32_t NetRUDP6ReadWrite( pal_socket_t sock, char *in, sint32_t il, char *out, sint32_t ol )
{
	return internal_send_recv(sock, in, il, out, ol);
}

/* */

sint32_t NetRUDP6Write( pal_socket_t sock, char *b, sint32_t l ) 
{
	return NetRUDPReadWrite(sock, b, l, NULL, 0);
}

/* */

sint32_t NetRUDP6Printf( pal_socket_t sock, char *out, sint32_t ol, char *Format, ... )
{
  va_list argp;
  sint32_t Length;
  char Data[1024];

  va_start(argp, Format);
  pal_vsnprintf(Data, sizeof Data, Format, argp);
  va_end(argp);

  Length = pal_strlen(Data);

  return NetRUDP6ReadWrite(sock, Data, pal_strlen(Data), out, ol);
}

/* */

sint32_t NetRUDP6Read( pal_socket_t sock, char *b, sint32_t l ) 
{
	return NetRUDP6ReadWrite(sock, NULL, 0, b, l);
}

static struct sockaddr_in6 *
internal_get_sai6(rttengine_stat_t *s, char *Host, uint16_t Port ) 
{
	/* we need to be reinitialised for each new connection,
	 * so we can check if we already have something
	 * cached and assume it is fit for the
	 * current situation
	 */

	struct sockaddr_in6 *sai;
	struct in6_addr addr6;

	/* so, is it cached? */
	
	if (s->sai != NULL)
		return (struct sockaddr_in6 *)s->sai;

	/* its not */
	
	/* get the IP address from the hostname */

	if(NetText2Addr6(Host, &addr6) == NULL )
			return NULL;

	/* get memory for our patente */
	if ( (sai = (struct sockaddr_in6 *)malloc(sizeof(struct sockaddr_in6))) == NULL)
		return NULL;
	
	/* clear out our sockaddr_in entry, fill it and cache it */

	memset(sai, 0, sizeof(struct sockaddr_in6));
	sai->sin6_family = PF_INET6;
	sai->sin6_port = htons(Port);
	memcpy(&sai->sin6_addr, &addr6, sizeof(struct in6_addr));
	s->sai = (struct sockaddr *)sai;

	return sai;
}

#endif /* V4V6_SUPPORT */
