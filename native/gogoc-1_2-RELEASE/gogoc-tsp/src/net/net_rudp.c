/*
-----------------------------------------------------------------------------
 $Id: net_rudp.c,v 1.1 2009/11/20 16:53:39 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"

#include "net_rudp.h"
#include "net.h"
#include "log.h"
#include "hex_strings.h"

// global variables

rttengine_stat_t rttengine_stats;

// forward declarations
static struct sockaddr_in *internal_get_sai(rttengine_stat_t *, char *, unsigned short);


/* Exported functions */

sint32_t NetRUDPInit(void) 
{
	memset(&rttengine_stats, 0, sizeof(rttengine_stat_t));
	return rttengine_init(&rttengine_stats);
}


/* */
sint32_t NetRUDPDestroy(void) 
{
	if ( rttengine_deinit(&rttengine_stats, NULL, NULL) == 0)
		return 1;
	return 0;
}


// --------------------------------------------------------------------------
// NetRUDPConnect:
//
// Return values:
//    0: success
//   -1: Failed to resolve srvname.
//   -2: Socket error / Failed to connect (TCP only).
//
sint32_t NetRUDPConnect(pal_socket_t *p_sock, char *Host, uint16_t Port) 
{
	pal_socket_t sfd;
	struct sockaddr_in *sai;

	if( rttengine_stats.initiated == 0 )
		NetRUDPInit();

  sai = internal_get_sai(&rttengine_stats, Host, Port);
	if( sai == NULL)
  {
		return -1;
	}

	/* and get a socket */
	if ( (sfd = pal_socket(PF_INET, SOCK_DGRAM, 0)) == -1 )
  {
		return -2;
	}

	/* then connect it */
	if( (pal_connect(sfd,(struct sockaddr *) sai, sizeof(struct sockaddr_in))) == -1 )
  {
    pal_closesocket( sfd );
		return -2;
	}
	
  *p_sock = sfd;
	return 0;
}


/* */
sint32_t NetRUDPClose(pal_socket_t sock) 
{
	pal_shutdown( sock, PAL_SOCK_SHTDN_BOTH );
	pal_closesocket( sock );
	return NetRUDPDestroy();
}


/* */
sint32_t NetRUDPReadWrite(pal_socket_t sock, char *in, sint32_t il, char *out, sint32_t ol)
{
	return internal_send_recv(sock, in, il, out, ol);
}


/* */
sint32_t NetRUDPWrite(pal_socket_t sock, char *b, sint32_t l) 
{
	return NetRUDPReadWrite(sock, b, l, NULL, 0);
}


/* */
sint32_t NetRUDPPrintf(pal_socket_t sock, char *out, sint32_t ol, char *Format, ...)
{
  va_list argp;
  char Data[1024];

  va_start(argp, Format);
  pal_vsnprintf((char*)Data, sizeof Data, Format, argp);
  va_end(argp);

  return NetRUDPReadWrite(sock, Data, pal_strlen(Data), out, ol);
}


/* */
sint32_t NetRUDPRead(pal_socket_t sock, char *b, sint32_t l) 
{
	return NetRUDPReadWrite(sock, NULL, 0, b, l);
}


/* Internal functions; not exported */
/* needs a connected UDP socket or else all hell will break loose */ 
sint32_t internal_send_recv(pal_socket_t fd, void *in, sint32_t il, void *out, sint32_t ol)
{
	fd_set fs;
	sint32_t ret, ls;	/* return code, length sent */
	rudp_msghdr_t *omh = NULL; /* outoing message header */
	rudp_msghdr_t *imh = NULL; /* incoming message header */
	void *om = NULL;
	void *im = NULL;       /* outgoing and incoming messages raw data */
	struct timeval tv_sel, tv_beg;
	

	if ( rttengine_stats.initiated == 0 )
		return -1;

	om = internal_prepare_message(&omh,il);
	im = internal_prepare_message(&imh,ol);

	memset(om, 0, il);
	memset(im, 0, ol);
	
	if (om == NULL || im == NULL) { /* something in the memory allocation failed */
		/* cleanup */
		rttengine_deinit(&rttengine_stats, om, im);
		return -1;
	}

	memcpy((char*)om+sizeof(rudp_msghdr_t), in, il);

	/* Bug 3334: Byte ordering is important when sending 32 bit
	 * values on the network: local and remote machines may not
	 * have the same ordering.  The rule observed here is that
	 * data going in and out the network MUST be in "network
	 * order".  The fix here is to apply this rule to the
	 * "timestamp" and "sequence" member of the RUDP header. The
	 * rudp_msghdr_t struct thus contains "network ordered"
	 * data */

	/* stamp in the sequence number */

	omh->sequence = htonl(rttengine_stats.sequence++ | 0xf0000000);
		

 sendloop: /* if we have no peer yet - that means retries = MAXRTT with no replies, quit it.
	    * if we do have a peer, then now is a good time to apply exponential backoff
	    */

	if (rttengine_stats.retries == RTTENGINE_MAXRTT) {
		if (rttengine_stats.has_peer == 0) {
			rttengine_deinit(&rttengine_stats, om, im);
			return -1;
		} else rttengine_stats.apply_backoff = 1;
	}
	
	
	if (rttengine_stats.retries == RTTENGINE_MAXRT) {
		/* cleanup */
		rttengine_deinit(&rttengine_stats, om, im);		
		return -1;
	}

	/* update the timestamp of the message */
	
	omh->timestamp = htonl(internal_get_timestamp(&rttengine_stats));

	Display(LOG_LEVEL_3, ELInfo, "internal_send_recv", GOGO_STR_RUDP_PACKET,rttengine_stats.retries, rttengine_stats.rto, ntohl(omh->sequence), ntohl(omh->timestamp));

        /* send the message */

	if ( ( ls = send(fd, om, il+sizeof(rudp_msghdr_t), 0)) == -1) {
		/* cleanup */
		rttengine_deinit(&rttengine_stats, om, im);
		return -1; /* if the send fails, quit it */ /* XXX check for a ICMP port unreachable here */
	}

	tv_sel.tv_sec=(sint32_t)rttengine_stats.rto; /* get the RTO in a format select can understand */
	tv_sel.tv_usec=(sint32_t)( (rttengine_stats.rto - tv_sel.tv_sec) * 1000 ); /* ie, 3.314 - 3 * 1000 = 314 milliseconds */

	pal_gettimeofday(&tv_beg);
	
 selectloop:
/* and wait for an answer */

	FD_ZERO(&fs);
	FD_SET(fd, &fs);

	ret = select((sint32_t)fd+1, &fs, NULL, NULL, &tv_sel);

	switch (ret) {

	case 0: {
		/* select timed out with nothing to read */
		/* so we might need to step back a little on the timeout, and do a resend */
		Display(LOG_LEVEL_3, ELInfo, "internal_send_recv", GOGO_STR_NO_RUDP_REPLY);
		if (rttengine_stats.apply_backoff == 1)
			rttengine_stats.rto = internal_get_adjusted_rto(rttengine_stats.rto *= 2);
		rttengine_stats.retries++;
		goto sendloop;
	}

	case 1: { /* We did get a reply, is it what we were waiting for?
		   * lets read everything the server is sending and see
		   */
		
		ret = recv(fd, im, sizeof(rudp_msghdr_t)+ol, 0);

		Display(LOG_LEVEL_3, ELInfo, "internal_send_recv", GOGO_STR_REPLY_RUDP_PACKET,rttengine_stats.retries, rttengine_stats.rto, ntohl(imh->sequence), ntohl(imh->timestamp));
		
		if (ret == -1) { /* fatal read error */
			/* cleanup */
			rttengine_deinit(&rttengine_stats, om, im);
			return -1;
		}
		
		if ( imh->sequence == omh->sequence ) {
			ret = ret - sizeof(rudp_msghdr_t);	/* we keep the lenght received minus the headers */
			break; /* yes it is what we are waiting for */
		} else {
			/* readjust tv to the remaining time */
			/* tv_sel = time_now - time_beginning */

			struct timeval tv_now;
			pal_gettimeofday(&tv_now);

			tv_sel.tv_sec -= (tv_now.tv_sec - tv_beg.tv_sec);
			tv_beg = tv_now;
			/* XXX substract the usec as well */

			goto selectloop;

		}
	}
		
	default: { /* error of unknown origin, ret contains the ERRNO compatible error released by select() */
		/* cleanup */
		rttengine_deinit(&rttengine_stats, om, im);
		return -1;
	}

	}//switch

	/* update our stat engine, the RTT and compute the new RTO */

	rttengine_update(&rttengine_stats, internal_get_timestamp(&rttengine_stats) - ntohl(imh->timestamp));

	/* get the reply in a safe place */

	memcpy(out, (char*)im+sizeof(rudp_msghdr_t), ret);
	
	/* free the memory */
	
	internal_discard_message(om);
	internal_discard_message(im);

	/* and *goodbye* */

	rttengine_stats.has_peer = 1;	/* we have a peer it seems */
	rttengine_stats.retries = 0;	/* next packet can retry like it wishes to */
	

	return ret;
}


/* */
sint32_t rttengine_init(rttengine_stat_t *s) 
{
	struct timeval tv;
	
	if (s == NULL)
		return 0;

	if (s->initiated == 1)
		return s->initiated;

	if ( pal_gettimeofday(&tv) == -1 )
		memset(&tv, 0, sizeof(struct timeval));

	s->rtt = 0;
	s->srtt = 0;
	s->rttvar = 0.50;
	s->rto = RTTENGINE_TMIN;

	/* Bug 3334: Implementation of client and server handles
	 * sequence number in host order (little endian on i386),
	 * which is broken (should be in network order).  This causes
	 * problems when client uses big endian format.
	 */

        /* On the wire, sequence number 0 in little endian format is
	 * 00 00 00 f0, where f0 is the MSB. In network order, this
	 * should be f0 00 00 00.
	 *
	 * In order to stay compatible with "old" HexOS versions, we
	 * initialize the sequence number to 240 (0xf0), which creates
	 * an initial sequence number of f0 00 00 f0. The following
	 * sequences are:
	 * f0 00 00 f1
	 * f0 00 00 f2
	 * f0 00 00 f3, ...
	 *
	 * A pre-4.0 HexOS server will interpret these numbers as
	 * little endian:
	 * f0 00 00 f0
	 * f1 00 00 f0
	 * f2 00 00 f0 ...
	 * If more than 16 sequence numbers are needed, this will not
	 * work: sequence number f0 00 01 00 will be dropped by HexOS
	 * since the 0xf RUDP "signature" is gone.
	 * Test with TSP using authenticated digest-md5 shows that 5
	 * sequence numbers are needed. Still have room for 11 more
	 * packet exchange (sequence incremented by client only)
	 *
	 * HexOS >= 4.0 should convert the sequence number from network
	 * order to host order, and should look for RUDP signature in
	 * both MSB and LSB (backward compatible with old clients).
	 */
	s->sequence = 240;
	s->retries = 0;
	s->last_recv_sequence = 0xBAD;
	s->initial_timestamp = tv.tv_sec;

	s->has_peer = 0;
	s->apply_backoff = 0;
	s->initiated = 1;

	return s->initiated;
}


/* */
sint32_t rttengine_deinit(rttengine_stat_t *s, void *im, void *om) 
{
	if (s->sai != NULL) {
		free(s->sai);
		s->sai = NULL;
	}

	if (im != NULL) {
		internal_discard_message(im);
		im = NULL;
	}

	if (om != NULL) {
		internal_discard_message(om);
		om = NULL;
	}
	
	s->initiated = 0;
	return s->initiated;
}


/* */
float rttengine_update(rttengine_stat_t *s, uint32_t rtt) 
{
	float delta;

	if (s == NULL)
		return 0; 

	s->rtt = rtt / 1000.0f;	/* bring this back to seconds, the rest of the code uses milliseconds, including the timestamp that is in the payload */
	delta = s->rtt - s->srtt;
	s->srtt = s->srtt + RTTENGINE_G * delta;
	s->rttvar = s->rttvar + RTTENGINE_H * ( delta<0?-delta:delta );
	s->rto = s->srtt + 4 * s->rttvar;
	s->rto = internal_get_adjusted_rto(s->rto);

	s->retries = 0; /* update is called when a packet was acked OK, so we need to reset this here */

	return s->rto;
}


/* */
void *internal_prepare_message(rudp_msghdr_t **hdr, size_t msglen) 
{
	void *buf;

	if (msglen == 0)
		buf = (rudp_msghdr_t *) malloc(sizeof(rudp_msghdr_t));
	else buf = (void *) malloc(msglen+sizeof(rudp_msghdr_t));

	if (buf!=NULL)
		*hdr = buf;
	return buf;
	
}


/* */
void internal_discard_message(void *m) 
{
	if (m != NULL)
		free(m);
	return;
}


/* */
uint32_t internal_get_timestamp(rttengine_stat_t *s)
{
	struct timeval tv;

	if (pal_gettimeofday(&tv) == -1)
		return 0;

	return ( ((tv.tv_sec - s->initial_timestamp) * 1000 ) + (tv.tv_usec / 1000) );
}


/* */
float internal_get_adjusted_rto(float rto) 
{
	if (rto<RTTENGINE_TMIN)
		return RTTENGINE_TMIN;
	if (rto>RTTENGINE_TMAX)
		return RTTENGINE_TMAX;

	return rto;
}


/* */
static struct sockaddr_in * internal_get_sai(rttengine_stat_t *s, char *Host, unsigned short Port) 
{
	/* we need to be reinitialised for each new connection,
	 * so we can check if we already have something
	 * cached and assume it is fit for the
	 * current situation
	 */

	struct sockaddr_in *sai;
	struct in_addr addr;

	/* so, is it cached? */
	
	if (s->sai != NULL)
		return (struct sockaddr_in *)s->sai;

	/* its not */
	
	/* get the IP address from the hostname */

	if(NetText2Addr(Host, &addr) == NULL )
			return NULL;

	/* get memory for our patente */
	if ( (sai = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in))) == NULL)
		return NULL;
	
	/* clear out our sockaddr_in entry, fill it and cache it */

	memset(sai, 0, sizeof(struct sockaddr_in));
	sai->sin_family = PF_INET;
	sai->sin_port = htons(Port);
	sai->sin_addr.s_addr = addr.s_addr;
	s->sai = (struct sockaddr *)sai;

	return sai;
}
