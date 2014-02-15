/*
-----------------------------------------------------------------------------
 dns.c - Dynamic DNS update library
-----------------------------------------------------------------------------
 $Id: dns.c,v 1.1 2009/11/20 16:53:37 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
----------------------------------------------------------------------------------
*/

#include "platform.h"
#include "net.h"
#include "net_tcp6.h"
#include "dns.h"

#define ADD16(b,s) {\
 register uint16_t *bb = (uint16_t *) b; \
 *bb = htons(s); \
 b += DNS16SZ; \
}

#define ADD32(b,s) {\
 register uint32_t *bb = (uint32_t *) b; \
 *bb = htonl(s); \
  b += DNS32SZ; \
}

#define GET16(b,s) {\
 s = ntohs(*((uint16_t *) b)); \
 b += DNS16SZ; \
}

#define GET32(b,s) {\
 s = ntohl(*((uint32_t *) b)); \
 b += DNS32SZ; \
}

/*
 * Generate a DNS id
 */
static uint16_t
DNSGenerateID(void)
{
	static uint16_t id = 0;
	struct timeval now;

	pal_gettimeofday(&now);

	if (!id)
		id = (uint16_t) (0xffff & (now.tv_sec ^ now.tv_usec ^ pal_getpid()));
	else
		id += 1 + (uint16_t)(now.tv_usec % 263);
	
	return id;
}

/*
 * Encode a DNS name in a DNS binary format
 *
 * *Pos_pp is the location inside the DNS message buffer where the name
 * must be written.  At the end, it's updated to point to the location right
 * after the written name.
 *
 * Offset is the compression offset to use.  A value of zero indicates no
 * compression.
 */
static tDNSRCode
DNSNameEncode(char **Pos_pp, char *Name, uint16_t Offset)
{
	char *Pos_p;
	char *Name_p;
	char *Dot_p;
	size_t Len;
	
	if (NULL == Pos_pp ||
	    NULL == *Pos_pp ||
	    NULL == Name ||
	    strlen(Name) > DNS_NAME_SIZE)
		return DNS_RCODE_FORMERR;
	
	Pos_p = *Pos_pp;
	Name_p = Name;

	while ('\0' != *Name_p) {
		/* Copy label */
		Dot_p = strchr(Name_p, '.');
		Len = (NULL != Dot_p) ? (size_t) (Dot_p - Name_p) : strlen(Name_p);
		if (Len) {
			/* Length of the label */ 
			*Pos_p++ = (uint8_t) Len;
			/* Label */
			memcpy(Pos_p, Name_p, Len);
			Pos_p += Len;
			Name_p += Len;
		}
		
		if (NULL != Dot_p)
			Name_p++; /* Skip dot */
	}

	if (Offset) {
		/* Compression offset */
		ADD16(Pos_p, 0xc000 | Offset);
	} else
		*Pos_p++ = 0;

	*Pos_pp = Pos_p;
		
	return DNS_RCODE_NOERROR;
}

/*
 * Create a DNS message.
 *
 * If AAAA is non NULL:
 *  - create a AAAA update message for Name.Domain with value AAAA.
 *
 * If AAAA is NULL:
 *  - create an update message for the deletion of all RRsets that belongs
 *    to Name.Domain.
 *
 * *Len_p is set with the length of the created message.
 *
 */
static tDNSRCode
DNSMessageCreate(char *Buffer, uint16_t DNSid, char *Domain, char *Name,  uint32_t TTL, char *AAAA, size_t *Len_p)
{
	char *p;
	size_t Domain_len;
	size_t Name_len;
	struct in6_addr AAAA_addr;
	struct in6_addr *AAAA_addr_p = NULL;
	tDNSRCode ret;

	if (NULL == Buffer ||
	    NULL == Name ||
	    NULL == Domain ||
	    NULL == Len_p)
		return DNS_RCODE_FORMERR;
	
	Name_len = strlen(Name);
	Domain_len = strlen(Domain);

	/* Validate Name.Domain length */
	if (Name_len + 1 + Domain_len > DNS_NAME_SIZE ||
	    Name_len > DNS_LABEL_SIZE)
		return DNS_RCODE_FORMERR;

	/* Convert AAAA name */
	if (NULL != AAAA) {
		AAAA_addr_p = NetText2Addr6(AAAA, &AAAA_addr);
		if (NULL == AAAA_addr_p)
			return DNS_RCODE_FORMERR;
	}
	

	/* Skip TCPMSGLENGTH (16 bits) - message length no known yet */
	p = Buffer + DNS16SZ;

	/* DNS id (16 bits) */
	ADD16(p, DNSid);
	
	/*
	 * Fields QR (1 bit) = 0, Opcode (4 bits) = UPDATE,
	 * Reserved (7 bits) = 0, RCode (4 bits) = 0.
	 */
	ADD16(p, DNS_OPCODE_UPDATE << 11);

	/* Zone count (16 bits) = 1 */
	ADD16(p, 1);

	/* Prerequisite count (16 bits) = 0 */
	ADD16(p, 0);

	/* Update count (16 bits) = 1 */
	ADD16(p, 1);

	/* Additionnal data count (16 bits) = 0 */
	ADD16(p, 0);
	
	/*
	 * ZONE section
	 */
	ret = DNSNameEncode(&p, Domain, 0);			/* ZNAME */
	if (DNS_RCODE_NOERROR != ret)
		return ret;
	ADD16(p, DNS_TYPE_SOA);					/* ZTYPE */
	ADD16(p, DNS_CLASS_IN);					/* CLASS */
	
	/*
	 * Ressource Record Section
	 *
	 * For the RRNAME, DNS compression is used.
	 * DNS_HEADER_SIZE is the offset of the Zone Name.
	 */
	ret = DNSNameEncode(&p, Name, DNS_HEADER_SIZE);		/* RRNAME */
	if (DNS_RCODE_NOERROR != ret)
		return ret;
	
	if ( NULL != AAAA_addr_p) {
		/* Addition of AAAA record */
		ADD16(p, DNS_TYPE_AAAA);			/* RRTYPE */
		ADD16(p, DNS_CLASS_IN);				/* RRCLASS */
		ADD32(p, TTL);					/* RRTTL */
		ADD16(p, DNS_TYPE_AAAA_SIZE);			/* RRDLENGTH */
		memcpy(p, AAAA_addr_p, DNS_TYPE_AAAA_SIZE);	/* RRDATA */
		p += DNS_TYPE_AAAA_SIZE;
	} else {
		/* Deletion of all RRSETS */
		ADD16(p, DNS_TYPE_ANY);				/* RRTYPE */
		ADD16(p, DNS_CLASS_ANY);			/* RRCLASS */
		ADD32(p, 0);					/* RRTTL */
		ADD16(p, 0);					/* RRDLENGTH */
	}
	
	/* Update *Len_p and the TCPMSGLENGTH field */
	*Len_p = p - Buffer;
	p = Buffer;
	ADD16(p, (u_short)(*Len_p) - DNS16SZ);
	
	return DNS_RCODE_NOERROR;
}

/*
 * Perform a DNS update for Name.Domain.
 *
 * A non NULL value for AAAA indicates the addition of a AAAA record
 * with value AAAA.
 *
 * A NULL value indicated the deletion of all Ressource Records associated
 * with Name.Domain.
 */
static tDNSRCode
DNSUpdate(pal_socket_t Socket, char *Name, char *Domain, char *AAAA)
{
	uint16_t DNSid;
	uint16_t DNSidReply;
	uint16_t Header;
	uint16_t QR;
	uint16_t OPCode;
	uint16_t RCode;
	tDNSRCode ret;
	char Buffer[DNS_BUFFER_SIZE];
	size_t Len;
	char *p;
	
	/* Create DNS id */
	DNSid = DNSGenerateID();
	
	/* Create DNS Update message */
	if (DNS_RCODE_NOERROR != (ret = DNSMessageCreate(Buffer, DNSid,
							 Domain, Name,
							 DNS_DEFAULT_TTL,
							 AAAA, &Len)))
		return ret;

	/* Send request to server */
	if (NetTCP6Write(Socket, Buffer, (sint32_t)Len) != (sint32_t)Len)
		return DNS_RCODE_SERVFAIL;

	/* Read response */
	Len = NetTCP6Read(Socket, Buffer, sizeof(Buffer));

	/*
	 * We expect the response to be sent in one packet.
	 * To be forward compatible, only the following is validated:
	 *  - response must be at least the size of the TCPMSGLENGTH field
	 *    plus the DNS header.
	 *  - DNSid must match.
	 *  - QR field must be set to 1.
	 *  - Opcode must be UPDATE (5).
	 */
	if (Len < DNS16SZ + DNS_HEADER_SIZE)
		return DNS_RCODE_SERVFAIL;

	/*
	 * Skip TCPMSGLENGTH, in the future we might have answer larger than
	 *  the buffer size.
	 */
	p = Buffer + DNS16SZ;

	/* Extract DNSid */
	GET16(p, DNSidReply);

	/* Extract QR, Opcode, and Return Code */
	GET16(p, Header);
	QR = (Header & 0x8000) >> 15;
	OPCode = (Header & 0x7800) >> 11;
	RCode = Header & 0x000f;

	/* Validation */
	if (DNSid != DNSidReply ||
	    QR != 1 ||
	    OPCode != DNS_OPCODE_UPDATE)
		return DNS_RCODE_SERVFAIL;
	
	/* Return Return Code */
	return (tDNSRCode) RCode;
}

/*
 * Start a DNS update session.
 */
pal_socket_t DNSUpdateConnect(char *Server)
{
  pal_socket_t sock;

  if( NetTCP6Connect( &sock, Server, DNS_UPDATE_PORT) == 0 )
    return sock;

  return -1;
}

/*
 * Close DNS update session.
 */
sint32_t
DNSUpdateClose(pal_socket_t Socket)
{
	return NetTCP6Close(Socket);
}

/*
 * Perform a DNS update to add a AAAA record for Name.Domain with value AAAA.
 */
tDNSRCode
DNSUpdateAddAAAA(pal_socket_t Socket, char *Name, char *Domain, char *AAAA)
{
	return DNSUpdate(Socket, Name, Domain, AAAA);
}

/*
 * Perform a DNS update to delete all Ressource Records that belongs
 * to Name.Domain.
 */ 
tDNSRCode
DNSUpdateDelRRSets(pal_socket_t Socket, char *Name, char *Domain)
{
	return DNSUpdate(Socket, Name, Domain, NULL);
}
