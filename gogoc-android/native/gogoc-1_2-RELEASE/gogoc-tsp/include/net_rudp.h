/*
-----------------------------------------------------------------------------
 $Id: net_rudp.h,v 1.1 2009/11/20 16:53:16 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

#ifndef _rudp_h_
#define _rudp_h_

#define RTTENGINE_G  (float)1/8
#define RTTENGINE_H  (float)1/4
#define RTTENGINE_TMIN  2
#define RTTENGINE_TMAX  30
#define RTTENGINE_MAXRTT 3
#define RTTENGINE_MAXRT  8

extern sint32_t     NetRUDPConnect        (pal_socket_t *, char *, uint16_t);
extern sint32_t     NetRUDPClose          (pal_socket_t);

extern sint32_t     NetRUDPReadWrite      (pal_socket_t, char *, sint32_t, char *, sint32_t);

extern sint32_t     NetRUDPWrite          (pal_socket_t, char *, sint32_t);
extern sint32_t     NetRUDPPrintf         (pal_socket_t, char *, sint32_t, char *, ...);

extern sint32_t     NetRUDPRead           (pal_socket_t, char *, sint32_t);


typedef struct rudp_message_struct {
  uint32_t sequence;
  uint32_t timestamp;
} rudp_msghdr_t;


typedef struct rttengine_statistics {
  /* connected udp host stats */
  struct sockaddr* sai;

  /* stat stats */

  float rtt;
  float srtt;
  float rttvar;
  float rto;

  /* timeline stats */
  uint32_t sequence;
  sint32_t retries;
  sint32_t last_recv_sequence;
  sint32_t initial_timestamp;
  sint32_t apply_backoff;
  sint32_t has_peer;
  sint32_t initiated;
} rttengine_stat_t;

extern rttengine_stat_t rttengine_stats;

/* rudp engine functions */
extern sint32_t     rttengine_init        (rttengine_stat_t *);
extern sint32_t     rttengine_deinit      (rttengine_stat_t *, void *, void *);
extern void *       internal_prepare_message(rudp_msghdr_t **, size_t);
extern void         internal_discard_message(void *);
extern float        rttengine_update      (rttengine_stat_t *, uint32_t);
extern uint32_t     internal_get_timestamp(rttengine_stat_t *);
extern float        internal_get_adjusted_rto(float);
extern sint32_t     internal_send_recv    (pal_socket_t, void *, sint32_t, void *, sint32_t);

#endif
