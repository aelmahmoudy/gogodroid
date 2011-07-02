/*
-----------------------------------------------------------------------------
 dns.h - Dynamic DNS update define
-----------------------------------------------------------------------------
 $Id: dns.h,v 1.1 2009/11/20 16:53:14 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

#ifndef _DNS_H_
#define _DNS_H_

#define DNS_UPDATE_PORT 	853
#define DNS16SZ 		2
#define DNS32SZ 		4

/*
 * RFC 1035 defines the maximum encoded name length as 255,
 * which means the ascii representation is limited to 253.
 */
#define DNS_NAME_SIZE 		253
#define DNS_LABEL_SIZE 		63
#define DNS_HEADER_SIZE 	12
#define DNS_BUFFER_SIZE 	512

/* The proxy-ddns will overwrite the TTL value */
#define DNS_DEFAULT_TTL 	3600

/* DNS opcode */
#define DNS_OPCODE_UPDATE 	5

/* DNS type */
#define DNS_TYPE_SOA 		6
#define DNS_TYPE_AAAA 		28
#define DNS_TYPE_AAAA_SIZE 	16
#define DNS_TYPE_ANY 		255

/* DNS class */
#define DNS_CLASS_IN 		1
#define DNS_CLASS_ANY 		255

typedef enum {
	DNS_RCODE_NOERROR 	= 0,
	DNS_RCODE_FORMERR	= 1,
	DNS_RCODE_SERVFAIL	= 2,
	DNS_RCODE_NXDOMAIN	= 3,
	DNS_RCODE_NOTIMP	= 4,
	DNS_RCODE_REFUSED	= 5,
	DNS_RCODE_YXDOMAIN	= 6,
	DNS_RCODE_YXRRSET	= 7,
	DNS_RCODE_NXRRSET	= 8,
	DNS_RCODE_NOTAUTH	= 9,
	DNS_RCODE_NOTZONE	= 10,
	DNS_RCODE_11		= 11,
	DNS_RCODE_12		= 12,
	DNS_RCODE_13		= 13,
	DNS_RCODE_14		= 14,
	DNS_RCODE_15		= 15
} tDNSRCode;

/* Public functions */
pal_socket_t        DNSUpdateConnect      (char *Server);
tDNSRCode           DNSUpdateAddAAAA      (pal_socket_t Socket, char *Name, char *Domain, char *AAAA);
tDNSRCode           DNSUpdateDelRRSets    (pal_socket_t Socket, char *Name, char *Domain);
sint32_t            DNSUpdateClose        (pal_socket_t Socket);

#endif /* _DNS_H_ */
