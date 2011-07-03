/*
-----------------------------------------------------------------------------
 $Id: icmp_echo_engine.c,v 1.1 2009/11/20 16:53:38 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2008 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT

Description:
  ICMP Echo Engine implementation.
-----------------------------------------------------------------------------
*/

#include "platform.h"

#include "icmp_echo_engine.h"
#include "net_cksm.h"                 // Used for ICMP header checksum.
#include "log.h"


#undef  MIN
#define MIN(X,Y)                      (((X)<(Y))?X:Y)
#define ICMP_LITTERAL                 ((p_engine->icmp_saf==AF_INET)?"ICMP":"ICMPv6")

#define ICMP4_ECHO_REQUEST_TYPE       8
#define ICMP4_ECHO_REPLY_TYPE         0
#define ICMP6_ECHO_REQUEST_TYPE       128
#define ICMP6_ECHO_REPLY_TYPE         129
#define ICMP6_ROUTER_ADVERTISEMENT    134 // Seen sometimes.
#define ICMP6_NEIGHBOR_SOLICITATION   135 // Seen sometimes.
#define ICMP6_NEIGHBOR_ADVERTISEMENT  136 // Seen sometimes.
#define ICMP_ECHO_CODE                0
#define ICMP_ECHO_DATA_LEN            sizeof(struct timeval)


// A note for displaying messages in this module:
//   This module was meant to return precise information to the utilizing
//   module. Only debugging messages should be logged(Display()ed) in this
//   module. Informational, warning and error messages should be logged in
//   the utilizing module, because this module is used by different
//   scenarios.
//   Logging such messages here could mess up the intended log message flow
//   of other scenario implementations.
#if defined(DEBUG) || defined(_DEBUG)
#ifndef __FUNCTION__
#define __FUNCTION__                  __func__
#endif
#define DBG_PRINT(X, ...)             Display( LOG_LEVEL_3, ELDebug, __FUNCTION__, X, ##__VA_ARGS__)
#else
#define DBG_PRINT(X, ...)
#endif


// --------------------------------------------------------------------------
// IPv4 and IPv6 ICMP ECHO [REQUEST and REPLY] header.
typedef struct __ICMP_ECHO_HEADER
{
  uint8_t         icmp_type;
  uint8_t         icmp_code;
  uint16_t        icmp_cksm;
  uint16_t        echo_id;
  uint16_t        echo_seq;
  uint8_t         echo_data[0];
} ICMP_ECHO_HEADER, * PICMP_ECHO_HEADER;


// --------------------------------------------------------------------------
// IPv6 pseudo header used for ICMPv6 checksum computation.
typedef struct __IP6_PSEUDO_HEADER
{
  struct in6_addr   ip6_src;        // source address
  struct in6_addr   ip6_dst;        // destination address
  uint32_t          hdr_len;        // Upper Layer Packet Length
  uint8_t           pad[3];         // - zeroed -
  uint8_t           nxt_hdr;        // IPPROTO_ICMPV6
  uint8_t           icmp_hdr[0];    // ICMP header.
} IP6_PSEUDO_HEADER, * PIP6_PSEUDO_HEADER;


// --------------------------------------------------------------------------
typedef struct __ECHO_EVENT
{
  uint32_t             echo_seq;    // Echo sequence.
  struct timeval       tv_timeout;  // Time at which this echo event times out.
  struct __ECHO_EVENT* next;        // Next chained echo event.
} ECHO_EVENT, * PECHO_EVENT;


// --------------------------------------------------------------------------
// ICMP echo engine parameters.
typedef struct __ICMP_ECHO_ENGINE_PARMS
{
  // Engine configurable parameters.
  uint32_t        send_interval;
  uint32_t        echo_num;
  uint32_t        echo_timeout;     // Not used in ACD.
  uint32_t        echo_timeout_threshold;
  uint8_t         eng_mode:2;       // Mode flag used by engine. OTHER | ACD | KA.

  // Engine processing status.
  uint8_t         eng_ongoing:1;    // Flag used by engine to know when to stop.

  // Engine statistical variables.
  uint32_t        count_send;       // Total number of echo requests sent.
  uint32_t        count_ontime;     // Total number of echo replies received on time.
  uint32_t        count_late;       // Total number of echo replies that were late.
  uint8_t         count_consec_late;// Number of consecutive late echo replies.

  // Engine echo event list.
  PECHO_EVENT     event_list;

  // Engine echo send and receive callbacks.
  iee_send_clbk   clbk_send;
  iee_recv_clbk   clbk_recv;

  // Engine socket variables.
  uint32_t        icmp_echo_id;     // ICMP ECHO identifier (this process id).
  pal_socket_t    icmp_sfd;         // ICMP raw socket file descriptor.
  sint32_t        icmp_saf;         // ICMP raw socket address family.
  union {
    struct sockaddr_in  in4;
    struct sockaddr_in6 in6;
  } echo_addr_src;                  // IPv4 or IPv6 source address.
  union {
    struct sockaddr_in  in4;
    struct sockaddr_in6 in6;
  } echo_addr_dst;                  // IPv4 or IPv6 destination address.

} ICMP_ECHO_ENGINE_PARMS, * PICMP_ECHO_ENGINE_PARMS;


// --------------------------------------------------------------------------
// IEE internal private statuses
typedef enum {
  READ_SELECT_ERROR,        // Returned by _do_read
  READ_SELECT_TIMEOUT,      // Returned by _do_read
  READ_SOCKET_CLOSED,       // Returned by _do_read
  READ_RECV_ERROR,          // Returned by _do_read

  SEND_PINGOUT_SUCCESS,     // Returned by _do_send
  SEND_PINGOUT_ERROR,       // Returned by _do_send
  SEND_MEMORY_STARVATION,   // Returned by _do_send

  ANAL_PACKET_BAD,          // Returned by _decode_icmp_packet & _do_read
  ANAL_PACKET_IGNORED,      // Returned by _decode_icmp_packet & _do_read
  ANAL_PACKET_PINGIN_LATE,  // Returned by _decode_icmp_packet & _do_read
  ANAL_PACKET_PINGIN_ONTIME,// Returned by _decode_icmp_packet & _do_read

  ECHO_EVENT_REMOVED,       // Returned by _remove_free_echo_event
  ECHO_EVENT_NOTFOUND       // Returned by _remove_free_echo_event
} iee_priv_ret_t;


// --------------------------------------------------------------------------
// Local private function prototypes.
iee_ret_t           _do_send_wrap         ( PICMP_ECHO_ENGINE_PARMS p_engine );
iee_priv_ret_t      _do_send              ( PICMP_ECHO_ENGINE_PARMS p_engine );
void                _calc_icmp_csum       ( PICMP_ECHO_ENGINE_PARMS p_engine, PICMP_ECHO_HEADER icmp_hdr );
iee_ret_t           _do_read_wrap         ( PICMP_ECHO_ENGINE_PARMS p_engine, struct timeval* tv_ref );
iee_priv_ret_t      _do_read              ( PICMP_ECHO_ENGINE_PARMS p_engine, struct timeval* tv_delay, uint32_t* echo_seq );
iee_priv_ret_t      _decode_icmp_packet   ( PICMP_ECHO_ENGINE_PARMS p_engine, uint8_t* pkt_data, uint32_t pkt_len, uint32_t* echo_seq );

PECHO_EVENT         _create_insert_echo_event( PICMP_ECHO_ENGINE_PARMS p_engine, struct timeval* tv_sent, uint32_t echo_seq );
void                _insert_echo_event    ( PICMP_ECHO_ENGINE_PARMS p_engine, PECHO_EVENT p_event_insert );
iee_priv_ret_t      _remove_free_echo_event( PICMP_ECHO_ENGINE_PARMS p_engine, uint32_t echo_seq );
void                _free_echo_event_list ( PECHO_EVENT p_event );
void                _compute_next_send    ( PICMP_ECHO_ENGINE_PARMS p_engine, struct timeval* tv_next_send );
void                _compute_echo_timeout ( PICMP_ECHO_ENGINE_PARMS p_engine, struct timeval* tv_timeout );
void                _conv_ms_to_tv        ( double ms, struct timeval* tv );
double              _compute_tv_diff_now  ( struct timeval* tv_diff );
double              _compare_tv           ( struct timeval* tv_1, struct timeval* tv_2 );


// --------------------------------------------------------------------------
// IEE_init: ICMP Echo Engine initialisation routine.
//
// Parameters:
//   pp_config: Opaque double pointer to a ICMP_ECHO_ENGINE_PARMS structure.
//   eng_mode: The mode of the ICMP Echo Engine.
//   send_interval: Fixed interval, in miliseconds, at which ICMP echo
//     requests will be issued.
//   echo_num: Number of ICMP echo requests to issue(0=infinite).
//   echo_timeout: Number of miliseconds after which an unanswered ICMP
//     echo request will be marked as timed out.
//   echo_timeout_threshold: Number of consecutive timed-out ICMP echo
//     requests to declare a general echo timeout.
//   src: Source address used for sending ICMP echo requests.
//   dst: Destination address at which ICMP echo requests will be sent.
//   family: address family (INET or INET6)
//
// Return values:
//   IEE_SUCCESS on success.
//   IEE_INVALID_PARMS if invalid pp_config.
//
iee_ret_t IEE_init( void** pp_config, iee_mode_t eng_mode,
                   uint32_t send_interval, uint32_t echo_num,
                   uint32_t echo_timeout, uint8_t echo_timeout_threshold,
                   char* src, char* dst, sint32_t af,
                   iee_send_clbk send_clbk, iee_recv_clbk recv_clbk )
{
  PICMP_ECHO_ENGINE_PARMS p_engine = NULL;


  // Verify input parameters.
  if( pp_config == NULL  ||  *pp_config != NULL  ||  dst == NULL )
  {
    // Error: bad input parameters.
    return IEE_INVALID_PARMS;
  }

  // Reserve memory for the engine parameters
  *pp_config = pal_malloc( sizeof(ICMP_ECHO_ENGINE_PARMS) );
  p_engine = (PICMP_ECHO_ENGINE_PARMS)*pp_config;
  if( p_engine == NULL )
  {
    // Error: Not enough memory for structure.
    return IEE_RESOURCE_STARVATION;
  }

  // Initialize engine structure with input parameters.
  memset( p_engine, 0, sizeof(ICMP_ECHO_ENGINE_PARMS) );
  p_engine->send_interval          = send_interval;
  p_engine->echo_num               = echo_num;
  p_engine->echo_timeout           = echo_timeout;    // Not used in ACD
  p_engine->echo_timeout_threshold = echo_timeout_threshold;
  p_engine->eng_mode               = eng_mode;

  // Initialize engine variables.
  p_engine->eng_ongoing       = 1;
  p_engine->count_send        = 0;
  p_engine->count_late        = 0;
  p_engine->count_ontime      = 0;
  p_engine->count_consec_late = 0;
  p_engine->event_list = NULL;

  // Set engine callback functions.
  p_engine->clbk_send = send_clbk;
  p_engine->clbk_recv = recv_clbk;

  // Initialize engine socket variables.
  p_engine->icmp_echo_id = pal_getpid();
  p_engine->icmp_saf = af;
  switch( p_engine->icmp_saf )
  {
  case AF_INET:
    if( pal_inet_pton( AF_INET, src, &p_engine->echo_addr_src.in4.sin_addr ) <= 0 )
    {
      // Bad IPv4 address in 'src'.
      pal_free( p_engine );
      return IEE_INVALID_PARMS;
    }
    p_engine->echo_addr_src.in4.sin_family = AF_INET;

    if( pal_inet_pton( AF_INET, dst, &p_engine->echo_addr_dst.in4.sin_addr ) <= 0 )
    {
      // Bad IPv4 address in 'dst'.
      pal_free( p_engine );
      return IEE_INVALID_PARMS;
    }
    p_engine->echo_addr_dst.in4.sin_family = AF_INET;

    p_engine->icmp_sfd = pal_socket( AF_INET, SOCK_RAW, IPPROTO_ICMP );
    break;

  case AF_INET6:
    if( pal_inet_pton( AF_INET6, src, &p_engine->echo_addr_src.in6.sin6_addr ) <= 0 )
    {
      // Bad IPv6 address in 'src'.
      pal_free( p_engine );
      return IEE_INVALID_PARMS;
    }
    p_engine->echo_addr_src.in6.sin6_family = AF_INET6;

    if( pal_inet_pton( AF_INET6, dst, &p_engine->echo_addr_dst.in6.sin6_addr ) <= 0 )
    {
      // Bad IPv6 address in 'dst'.
      pal_free( p_engine );
      return IEE_INVALID_PARMS;
    }
    p_engine->echo_addr_dst.in6.sin6_family = AF_INET6;

    p_engine->icmp_sfd = pal_socket( AF_INET6, SOCK_RAW, IPPROTO_ICMPV6 );
    break;

  default:
    // ERROR! Bad address family!
    pal_free( p_engine );
    return IEE_INVALID_PARMS;
  }

  // Verify that the socket created is valid.
  if( p_engine->icmp_sfd == -1 )
  {
    // Failed to open a socket descriptor.
    pal_free( p_engine );
    return IEE_GENERAL_ECHO_ERROR;
  }

  return IEE_SUCCESS;
}

// --------------------------------------------------------------------------
// IEE_destroy: ICMP Echo Engine destruction routine.
//
// Parameters:
//   pp_config: Opaque double pointer to a ICMP_ECHO_ENGINE_PARMS structure.
//
// Return values:
//   IEE_SUCCESS on success.
//   IEE_INVALID_PARMS if invalid pp_config.
//
iee_ret_t IEE_destroy( void** pp_config )
{
  PICMP_ECHO_ENGINE_PARMS p_engine = NULL;


  // Verify input parameters.
  if( pp_config == NULL  || *pp_config == NULL )
  {
    // Error: invalid p_config, or already freed.
    return IEE_INVALID_PARMS;
  }

  // Cast opaque double pointer to allow manipulation.
  p_engine = (PICMP_ECHO_ENGINE_PARMS)*pp_config;

  // Close the ICMP raw socket.
  pal_closesocket( p_engine->icmp_sfd );

  // Free the engine echo event list.
  _free_echo_event_list( p_engine->event_list );

  // Deallocate the memory used by the engine structure.
  pal_free( p_engine );
  *pp_config = NULL;

  return IEE_SUCCESS;
}


// --------------------------------------------------------------------------
// IEE_process: ICMP Echo Engine main processing routine.
//
// Parameters:
//   p_config: Opaque pointer to a ICMP_ECHO_ENGINE_PARMS structure.
//
// Return values:
//   IEE_SUCCESS on normal execution.
//   IEE_INVALID_PARMS if invalid p_config.
//   IEE_CONNECTIVITY_ASSESSED (ACD only) Received one echo reply on time.
//   IEE_GENERAL_ECHO_TIMEOUT when the maximal number of successive timeouts
//               has been detected.
//   IEE_GENERAL_ECHO_ERROR on a fatal error.
//
iee_ret_t IEE_process( void* p_config )
{
  PICMP_ECHO_ENGINE_PARMS p_engine = (PICMP_ECHO_ENGINE_PARMS)p_config;
  struct timeval tv_reference;
  iee_ret_t retval = IEE_SUCCESS;


  // Verify input parameters.
  if( p_engine == NULL )
  {
    // Error: invalid p_config, or already freed.
    return IEE_INVALID_PARMS;
  }

  if( (iee_mode_t)p_engine->eng_mode == IEE_MODE_KA )
  {
    // When icmp echo engine is is Keepalive(KA) mode, the first sent echo
    // REQUEST is sent after a full interval. => Wait for one interval.
    uint32_t total_sleep_time = p_engine->send_interval;
    uint32_t sleep_time;

    DBG_PRINT("Sleeping %d milliseconds before sending first ECHO REQUEST.\n", total_sleep_time);

    // Break the sleep in several chunks in case we're notified to stop.
    while( p_engine->eng_ongoing == 1  &&  total_sleep_time > 0 )
    {
      sleep_time = (total_sleep_time>1000)?1000:total_sleep_time;
      pal_sleep( sleep_time );
      total_sleep_time -= sleep_time;
    }
  }


  // ------------------------------------------------------------------------
  // Main ICMP echo engine loop. Loop until we're notified to stop, or we've
  //   sent the number of ECHO REQUESTS we had to.
  // ------------------------------------------------------------------------
  while( p_engine->eng_ongoing == 1  &&
        (p_engine->echo_num == 0  ||  p_engine->count_send < p_engine->echo_num) )
  {
    // ------------------------------
    // Time to send an ECHO request.
    // ------------------------------
    retval = _do_send_wrap( p_engine );
    if( retval != IEE_SUCCESS )
    {
      // An error occurred while sending.
      break;
    }

    // Check if we've been notified to stop, or an event has triggered a stop.
    if( p_engine->eng_ongoing == 0 )
    {
      // Stop processing.
      break;
    }

    // Synchronize time reference variable with 'now'.
    pal_gettimeofday( &tv_reference );

    // Add one echo interval to tv_reference.
    _compute_next_send( p_engine, &tv_reference );

    // -------------------------------------------------------------------
    // Wait and read for incoming packets.
    //   Function will return when tv_reference is approximatively 'now'.
    // -------------------------------------------------------------------
    retval = _do_read_wrap( p_engine, &tv_reference );
    if( retval != IEE_SUCCESS )
    {
      // A retval different than IEE_SUCCESS indicates that we should stop
      //   processing. It does not necessarily mean an error occurred.
      break;
    }
    DBG_PRINT("\n");
  }

  DBG_PRINT("Statistics:\n\tSent: %03d\n\tReceived on time: %03d\n\tReceived late: %03d\n",
             p_engine->count_send, p_engine->count_ontime, p_engine->count_late );


  return retval;
}


// --------------------------------------------------------------------------
// IEE_stop: ICMP Echo Engine stop procedure. Stops the IEE_process function.
//           This function should be called from an external thread.
//
// Parameters:
//   p_config: Opaque pointer to a ICMP_ECHO_ENGINE_PARMS structure.
//
// Return values:
//   IEE_SUCCESS on success.
//   IEE_INVALID_PARMS if invalid pp_config.
//
iee_ret_t IEE_stop( void* p_config )
{
  PICMP_ECHO_ENGINE_PARMS p_engine = (PICMP_ECHO_ENGINE_PARMS)p_config;

  // Verify input parameters.
  if( p_engine == NULL )
  {
    // Error: invalid p_config, or already freed.
    return IEE_INVALID_PARMS;
  }

  // Notify the engine to stop.
  p_engine->eng_ongoing = 0;

  // Close the ICMP socket.
  //   If the ICMP echo engine is currently in a read operation, the
  // IEE_process() function will not return until the call to select()
  // returns. This time lapse depends on the current processing. It can take
  // up to p_engine->send_interval milliseconds. To break the select() call,
  // we close the ICMP socket so it will return as an error.
  //
  pal_closesocket( p_engine->icmp_sfd );  // break a wait on 'select()'.

  return IEE_SUCCESS;
}


// --------------------------------------------------------------------------
// _do_send_wrap: Private function used to wrap the actual write operation
//                and analyze the return code from the send to translate it
//                in a more general return code.
//
// Parameters:
//   p_engine: p_config: Pointer to an ICMP_ECHO_ENGINE_PARMS structure.
//
// Return values:
//   IEE_SUCCESS: Operation was successful. Continue.
//   IEE_GENERAL_ECHO_ERROR: Fatal error occurred. Abort.
//
iee_ret_t _do_send_wrap( PICMP_ECHO_ENGINE_PARMS p_engine )
{
  iee_priv_ret_t priv_retval;
  iee_ret_t retval;


  // Send an ICMP ECHO request.
  priv_retval = _do_send( p_engine );
  switch( priv_retval )
  {
  case SEND_PINGOUT_SUCCESS:
    // No error.
    retval = IEE_SUCCESS;
    break;

  case SEND_MEMORY_STARVATION:
    // Failed to acquire memory.
    retval = IEE_RESOURCE_STARVATION;
    break;

  case SEND_PINGOUT_ERROR:
  default:
    // An error occurred while sending the echo request.
    retval = IEE_GENERAL_ECHO_ERROR;
    break;
  }

  return retval;
}


// --------------------------------------------------------------------------
// _do_send: Private function used to send an ICMP ECHO REQUEST.
//   Creates an ICMP ECHO REQUEST packet and sends it. Creates an echo event
//   and inserts it in the engine's list of echo events.
//
// Parameters:
//   p_engine: Pointer to a ICMP_ECHO_ENGINE_PARMS structure.
//
// Return values:
//   IEE_SUCCESS when no errors happened.
//   IEE_GENERAL_ECHO_ERROR on fatal error.
//
iee_priv_ret_t _do_send( PICMP_ECHO_ENGINE_PARMS p_engine )
{
  PICMP_ECHO_HEADER icmp_hdr;
  uint8_t send_buf[sizeof(ICMP_ECHO_HEADER) + ICMP_ECHO_DATA_LEN];
  const uint16_t send_buf_len = sizeof(ICMP_ECHO_HEADER) + ICMP_ECHO_DATA_LEN;
  iee_priv_ret_t retval = SEND_PINGOUT_SUCCESS;
  sint32_t ret;


  // Map the ICMP header unto the send buffer.
  icmp_hdr = (PICMP_ECHO_HEADER)send_buf;

  if( p_engine->icmp_saf == AF_INET )
  {
    icmp_hdr->icmp_type = ICMP4_ECHO_REQUEST_TYPE;
  }
  else
  {
    icmp_hdr->icmp_type = ICMP6_ECHO_REQUEST_TYPE;
  }
  icmp_hdr->icmp_code = ICMP_ECHO_CODE;
  icmp_hdr->icmp_cksm = 0;
  icmp_hdr->echo_id = p_engine->icmp_echo_id;
  icmp_hdr->echo_seq = (p_engine->count_send)++;   // Starts at 0.
  pal_gettimeofday( (struct timeval*)(icmp_hdr->echo_data) );

  // Calculate the ICMP header checksum.
  _calc_icmp_csum( p_engine, icmp_hdr );


  // Create the echo event, and insert it in the echo engine sorted echo
  //   event list.
  if( _create_insert_echo_event( p_engine,
        (struct timeval*)(icmp_hdr->echo_data), icmp_hdr->echo_seq ) != NULL )
  {
    // Send the ICMP packet.
    DBG_PRINT(">> %s ECHO REQUEST, id:%d, seq:%d, len:%d...\n",
               ICMP_LITTERAL, icmp_hdr->echo_id, icmp_hdr->echo_seq, send_buf_len );
    ret = sendto( p_engine->icmp_sfd, send_buf, send_buf_len, 0, (struct sockaddr*)&(p_engine->echo_addr_dst), sizeof(p_engine->echo_addr_dst) );
    if( ret != send_buf_len )
    {
      // The return value of 'sendto()' did not match what was expected. Error.
      retval = SEND_PINGOUT_ERROR;
      DBG_PRINT("Failed to send ICMP REQUEST packet. Error code:%d\n", ret);
    }
    else if( p_engine->clbk_send != NULL )
    {
      p_engine->clbk_send();
    }
  }
  else
  {
    // Resource starvation. Not enough memory.
    retval = SEND_MEMORY_STARVATION;
    DBG_PRINT("Failed to allocate memory for echo event.\n");
  }

  return retval;
}


// --------------------------------------------------------------------------
// _calc_icmp_csum: Calculates the ICMP checksum. The checksum is stored in
//   the ICMP header passed in.
//
// Parameters:
//   p_engine: p_config: Pointer to an ICMP_ECHO_ENGINE_PARMS structure.
//   icmp_hdr: ICMP packet header.
//
// Return value: (none)
//
void _calc_icmp_csum( PICMP_ECHO_ENGINE_PARMS p_engine, PICMP_ECHO_HEADER icmp_hdr )
{
  PIP6_PSEUDO_HEADER ip6_pseudo;
  uint8_t pseudo_buf[sizeof(IP6_PSEUDO_HEADER) + sizeof(ICMP_ECHO_HEADER) + ICMP_ECHO_DATA_LEN];
  const uint16_t pseudo_buf_len = sizeof(pseudo_buf);


  // From RFC 2460:
  //   The IPv6 version of ICMP [ICMPv6] includes the above pseudo-header in
  // its checksum computation; this is a change from the IPv4 version of
  // ICMP, which does not include a pseudo-header in its checksum.  The
  // reason for the change is to protect ICMP from misdelivery or
  // corruption of those fields of the IPv6 header on which it depends,
  // which, unlike IPv4, are not covered by an internet-layer checksum.
  // The Next Header field in the pseudo-header for ICMP contains the
  // value 58, which identifies the IPv6 version of ICMP.
  if( p_engine->icmp_saf == AF_INET6 )
  {
    memset( pseudo_buf, 0, pseudo_buf_len );

    // Map the IP6 pseudo header on the pseudo_buf.
    ip6_pseudo = (PIP6_PSEUDO_HEADER)pseudo_buf;

    // Fill in pseudo header data.
    memcpy( &ip6_pseudo->ip6_src, &p_engine->echo_addr_src.in6.sin6_addr, sizeof(ip6_pseudo->ip6_src) );
    memcpy( &ip6_pseudo->ip6_dst, &p_engine->echo_addr_dst.in6.sin6_addr, sizeof(ip6_pseudo->ip6_dst) );
    ip6_pseudo->hdr_len = htonl( sizeof(ICMP_ECHO_HEADER) + ICMP_ECHO_DATA_LEN );
    ip6_pseudo->nxt_hdr = IPPROTO_ICMPV6;

    // Copy the ICMP header in the pseudov6 header.
    memcpy( ip6_pseudo->icmp_hdr, icmp_hdr, sizeof(ICMP_ECHO_HEADER) + ICMP_ECHO_DATA_LEN );

    // Calculate the ICMP checksum.
    icmp_hdr->icmp_cksm = in_cksum( (uint16_t*)ip6_pseudo, pseudo_buf_len );
  }
  else
  {
    // Calculate the ICMP checksum.
    icmp_hdr->icmp_cksm = in_cksum( (uint16_t*)icmp_hdr, sizeof(ICMP_ECHO_HEADER) + ICMP_ECHO_DATA_LEN );
  }
}


// --------------------------------------------------------------------------
// _do_read_wrap: Private function used to wrap the actual read operation and
//    analyse the return code from the read to translate it in a more general
//    return code.
//
// Parameters:
//   p_engine: p_config: Pointer to an ICMP_ECHO_ENGINE_PARMS structure.
//   tv_ref: Absolute time at which we should exit this function.
//
// Return values:
//   IEE_SUCCESS: Operation was successful. Continue.
//   IEE_GENERAL_ECHO_ERROR: Fatal error occurred. Abort.
//   IEE_CONNECTIVITY_ASSESSED: (ACD only) Stop processing.
//
iee_ret_t _do_read_wrap( PICMP_ECHO_ENGINE_PARMS p_engine, struct timeval* tv_ref )
{
  double time_left_ms;          // Time left to spend in this function.
  double read_delay;            // Number of milliseconds before the soonest echo event times out.
  struct timeval tv_delay;      // The actual time delay.
  uint32_t echo_seq_read;       // Echo sequence read.
  iee_priv_ret_t priv_retval;
  iee_ret_t retval = IEE_SUCCESS;


  // Check how much time we have left to spend here.
  time_left_ms = _compute_tv_diff_now( tv_ref );
  if( time_left_ms <= 0.0 )
  {
    // Return because it's already time to send a new ECHO REQUEST.
    // Should not happen in normal processing. Can happen if debugging.
    return IEE_SUCCESS;
  }


  // Main read loop.
  do
  {
    if( p_engine->event_list != NULL )
    {
      // Check how much time before the soonest echo event times out.
      read_delay = _compute_tv_diff_now( &p_engine->event_list->tv_timeout );
    }
    else
    {
      read_delay = p_engine->send_interval;
    }
    // Translate that in a timeval structure.
    _conv_ms_to_tv( MIN(time_left_ms, read_delay), &tv_delay );

    // ------------------------------------------
    // Wait for an incoming packet, and read it.
    // ------------------------------------------
    priv_retval = _do_read( p_engine, &tv_delay, &echo_seq_read );

    // Perform operations depending on what happened in the read.
    switch( priv_retval )
    {
      case ANAL_PACKET_BAD:
      case ANAL_PACKET_IGNORED:
        // We don't care.
        break;

      case READ_SELECT_TIMEOUT:
        if( time_left_ms > read_delay  &&  p_engine->event_list != NULL )
        {
          // An echo event has timed out.
          // => Increment late counter and consecutive late counter.
          p_engine->count_late++;
          p_engine->count_consec_late++;
          DBG_PRINT("--> Echo timeout detected! count_consec_late:%d\n",p_engine->count_consec_late);
          // => Remove the echo event from the list.
          _remove_free_echo_event( p_engine, p_engine->event_list->echo_seq );
        }
        // else, we have reached the time to quit processing incoming
        //   packets.
        break;

      case ANAL_PACKET_PINGIN_ONTIME:
        // We received an ECHO REPLY on time.
        // => Reset the consecutive late counter. Increment the ontime counter.
        p_engine->count_consec_late = 0;
        p_engine->count_ontime++;
        // => Remove the associated echo event from the list.
        priv_retval = _remove_free_echo_event( p_engine, echo_seq_read );
        if( priv_retval != ECHO_EVENT_REMOVED )
        {
          // This event was already removed from the list (Probably because
          //   a READ_SELECT_TIMEOUT occured while waiting for it).
          // Since it is a valid on time reply, we've reset the consecutive
          //   late count.
          p_engine->count_late--;
          DBG_PRINT("--> Last echo timeout cancelled.\n");
        }

        // If the icmp echo engine is in mode Automatic Connectivity
        // Detection (ACD), any received reply assesses a valid
        // connectivity.
        if( (iee_mode_t)p_engine->eng_mode == IEE_MODE_ACD )
        {
          p_engine->eng_ongoing = 0;
          retval = IEE_CONNECTIVITY_ASSESSED;
          DBG_PRINT("Connectivity has been assessed (ACD).\n");
        }

        // ** INTENTIONAL FALLTHROUGH ** //

      case ANAL_PACKET_PINGIN_LATE:
        // The associated echo event should have already beed removed from
        //   the list of echo events.

        // Check if this received ECHO REPLY was the last.
        if( p_engine->echo_num != 0  &&  p_engine->count_send >= p_engine->echo_num )
        {
          // This is our last echo reply.
          p_engine->eng_ongoing = 0;
        }
        break;

      case READ_SOCKET_CLOSED:
        p_engine->eng_ongoing = 0;  // Should have already been set.
        retval = IEE_SUCCESS;
        DBG_PRINT("ICMP echo engine socket has been closed.");
        break;

      case READ_SELECT_ERROR:     // Fatal error
      case READ_RECV_ERROR:       // Fatal error
      default:                    // Unhandled cases (Should not occur).
        p_engine->eng_ongoing = 0;
        retval = IEE_GENERAL_ECHO_ERROR;
        break;
    }

    // Check if we've reached the maximal number of consecutive timeouts.
    if( p_engine->count_consec_late >= p_engine->echo_timeout_threshold )
    {
      retval = IEE_GENERAL_ECHO_TIMEOUT;
      p_engine->eng_ongoing = 0;
      DBG_PRINT("General Echo Timeout detected.\n");
    }

    // Check how much time we have left to spend here.
    time_left_ms = _compute_tv_diff_now( tv_ref );
  }
  // Loop until we have to stop reading packets, or we've been notified
  // to stop processing.
  while( time_left_ms > 0.0  &&  p_engine->eng_ongoing == 1 );


  return retval;
}


// --------------------------------------------------------------------------
// _do_read: private function used to wait/read on the ICMP socket. If an
//   ICMP packet is read, it is given to the analysis function.
//
// Parameters:
//   p_engine: Pointer to a ICMP_ECHO_ENGINE_PARMS structure.
//   tv_delay : Time to wait for an incoming packet.
//   echo_seq: Emplacement where the read echo sequence will be stored.
//
// Return values:
//   IEE_SUCCESS when no errors happened.
//   IEE_GENERAL_ECHO_ERROR on fatal error.
//
iee_priv_ret_t _do_read( PICMP_ECHO_ENGINE_PARMS p_engine, struct timeval* tv_delay, uint32_t *echo_seq )
{
  uint8_t read_buf[2048];
  fd_set fs;
  sint32_t ret;
  iee_priv_ret_t retval;


  FD_ZERO( &fs );
  FD_SET( p_engine->icmp_sfd, &fs );

  DBG_PRINT("Will wait for %.3f milliseconds.\n",
             ((double)(tv_delay->tv_sec * 1000)) + ((double)tv_delay->tv_usec) / 1000.0);

  // Wait on ICMP socket for incoming packet.
  ret = select( (sint32_t)p_engine->icmp_sfd + 1, &fs, NULL, NULL, tv_delay );
  switch( ret )
  {
  case 0:     // There was no packet available to read in the time lapse.
    retval = READ_SELECT_TIMEOUT;
    break;

  case 1:     // Data is available for read - or socket closed.
    ret = recv( p_engine->icmp_sfd, read_buf, sizeof(read_buf), 0 );
    if( ret > 0 )
    {
      // Analyse read packet.
      retval = _decode_icmp_packet( p_engine, read_buf, ret, echo_seq );
    }
    else if( ret == -1  &&  p_engine->eng_ongoing == 0 )
    {
      // Socket has been closed because we're exiting. See KA_stop().
      retval = READ_SOCKET_CLOSED;
    }
    else
    {
      // Abnormal return value from 'recv()'. Error.
      retval = READ_RECV_ERROR;
      DBG_PRINT("Error occurred while receiving a packet. Error code:%d\n", ret);
    }
    break;

  default:
    // On Linux/Darwin/*BSD, a signal can cause an interruption in the select
    //   routine.
    if( errno == EINTR )
    {
      // A signal has been received and has interrupted the select().
      retval = READ_SELECT_TIMEOUT;
      DBG_PRINT("Call to select() has been interrupted by a signal. Continuing.");
    }
    else if( errno == EBADF  &&  p_engine->eng_ongoing == 0 )
    {
      // Socket has been closed because we're exiting. See KA_stop().
      retval = READ_SOCKET_CLOSED;
    }
    else
    {
      // Abnormal return value from 'select()'. Error.
      retval = READ_SELECT_ERROR;
      DBG_PRINT("Error occurred while waiting for a packet. Error code:%d\n", ret);
    }
  }

  return retval;
}


// --------------------------------------------------------------------------
// _decode_icmp_packet: private function used to analyse an incoming ICMP
//   packet.
//
//  IMPORTANT NOTE:
//    IPv6 and IPv4 operate differently when receiving a socket with a type
//    of SOCK_RAW. The IPv4 receive packet includes the packet payload, the
//    next upper-level header (for example, the IP header for an ICMP
//    packet), and the IPv4 packet header. The IPv6 receive packet includes
//    the packet payload and the next upper-level header. The IPv6 receive
//    packet never includes the IPv6 packet header.
//
// Parameters:
//   p_engine: Pointer to a ICMP_ECHO_ENGINE_PARMS structure.
//   pkt_data: ICMP packet data.
//   pkt_len : ICMP packet length.
//   echo_seq: Pointer where the echo sequence received will be stored.
//
// Return values:
//   IEE_SUCCESS when no errors happened.
//   IEE_GENERAL_ECHO_ERROR on fatal error.
//
iee_priv_ret_t _decode_icmp_packet( PICMP_ECHO_ENGINE_PARMS p_engine, uint8_t* pkt_data, uint32_t pkt_len, uint32_t *echo_seq )
{
  PICMP_ECHO_HEADER icmp_hdr = NULL;  // ICMP header pointer.
  uint8_t ip_ver, ip_len=0;             // IP header version and length.
  double rtt;                         // Computed roundtrip time.
  iee_priv_ret_t priv_retval = ANAL_PACKET_PINGIN_ONTIME;


  do // Dummy loop used to 'break'.
  {
    if( p_engine->icmp_saf == AF_INET )
    {
      // Retrieve this packet IP version. Assert IP header IP version.
      ip_ver = (pkt_data[0] & 0xF0) >> 4;
      if( ip_ver != 0x04 )
      {
        // Packet IP address family does not match opened socket.
        priv_retval = ANAL_PACKET_BAD;
        DBG_PRINT("Invalid IP packet for address family. IP version:%d\n", ip_ver);
        break;
      }

      // Retrieve IP header length (found in bytes 4..7).
      ip_len = (pkt_data[0] & 0x0F) << 2;

      // Verify if packet length includes the IP header AND the ICMP header.
      if( pkt_len - ip_len < sizeof(ICMP_ECHO_HEADER) )
      {
        // This packet is too small to be an ICMP packet.
        priv_retval = ANAL_PACKET_BAD;
        DBG_PRINT("Packet is too small to be an ICMP packet. ICMP length:%d\n", pkt_len - ip_len);
        break;
      }

      // Cast raw packet data to the ICMP echo header.
      icmp_hdr = (PICMP_ECHO_HEADER)(pkt_data + ip_len);

      // Check ICMP type
      switch( icmp_hdr->icmp_type )
      {
      case ICMP4_ECHO_REPLY_TYPE:
        // Received an ECHO REPLY. Good.
        break;

      case ICMP4_ECHO_REQUEST_TYPE:
        // Received an ECHO REQUEST. We're not replying.
        priv_retval = ANAL_PACKET_IGNORED;
        DBG_PRINT("<< ICMP ECHO REQUEST, id:%d, seq:%d, len:%d\n",
                   icmp_hdr->echo_id, icmp_hdr->echo_seq, pkt_len - ip_len );
        break;

      default:
        // We have an unknown ICMP packet type. Do nothing.
        priv_retval = ANAL_PACKET_IGNORED;
        DBG_PRINT("Received unknown ICMP packet type: %d, code:%d, len:%d\n",
                   icmp_hdr->icmp_type, icmp_hdr->icmp_code, pkt_len - ip_len);
        break;
      }
    }
    else if( p_engine->icmp_saf == AF_INET6 )
    {
      // NOTE: On IPV6 raw sockets, the IPv6 is not included in the packet
      //       data.

      // Verify if packet length is big enough to be an ICMP packet.
      if( pkt_len < sizeof(ICMP_ECHO_HEADER) )
      {
        // This packet is too small to be an ICMP packet.
        priv_retval = ANAL_PACKET_BAD;
        DBG_PRINT("Packet is too small to be an ICMP packet. ICMP length:%d\n", pkt_len);
        break;
      }

      // Cast raw packet data to the ICMP echo header.
      icmp_hdr = (PICMP_ECHO_HEADER)pkt_data;

      // Check ICMP type
      switch( icmp_hdr->icmp_type )
      {
      case ICMP6_ECHO_REPLY_TYPE:
        // Received an ECHO REPLY. Good.
        break;

      case ICMP6_ECHO_REQUEST_TYPE:
        // Received an ECHO REQUEST. We're not replying.
        priv_retval = ANAL_PACKET_IGNORED;
        DBG_PRINT("<< ICMPv6 ECHO REQUEST, id:%d, seq:%d, len:%d\n",
                   icmp_hdr->echo_id, icmp_hdr->echo_seq, pkt_len );
        break;

      case ICMP6_ROUTER_ADVERTISEMENT:
        // These are sometimes received on the ICMP socket. Logged for informational purposes only.
        priv_retval = ANAL_PACKET_IGNORED;
        DBG_PRINT("<< ICMPv6 ROUTER ADVERTISEMENT, code:%d, len:%d\n",
                   icmp_hdr->icmp_code, pkt_len );
        break;

      case ICMP6_NEIGHBOR_SOLICITATION:
        // These are sometimes received on the ICMP socket. Logged for informational purposes only.
        priv_retval = ANAL_PACKET_IGNORED;
        DBG_PRINT("<< ICMPv6 NEIGHBOR SOLICITATION, code:%d, len:%d\n",
                   icmp_hdr->icmp_code, pkt_len );
        break;

      case ICMP6_NEIGHBOR_ADVERTISEMENT:
        // These are sometimes received on the ICMP socket. Logged for informational purposes only.
        priv_retval = ANAL_PACKET_IGNORED;
        DBG_PRINT("<< ICMPv6 NEIGHBOR ADVERTISEMENT, code:%d, len:%d\n",
                   icmp_hdr->icmp_code, pkt_len );
        break;

      default:
        // We have an unknown ICMP packet type. Do nothing.
        priv_retval = ANAL_PACKET_IGNORED;
        DBG_PRINT("Received unknown ICMPv6 packet type: %d, len:%d\n",
                   icmp_hdr->icmp_type, pkt_len );
        break;
      }
    }

    if( priv_retval != ANAL_PACKET_PINGIN_ONTIME )
    {
      // Packet is not an ICMP reply. Do not analyze it any further.
      break;
    }

    // Assert the ICMP code.
    if( icmp_hdr->icmp_code != ICMP_ECHO_CODE )
    {
      // If the type is an ECHO reply, the code should be ICMP_ECHO_CODE.
      priv_retval = IEE_GENERAL_ECHO_ERROR;
      DBG_PRINT("Invalid ICMP code for ECHO REPLY. ICMP code:%d\n", icmp_hdr->icmp_code);
      break;
    }

    // Assert the ICMP echo ID.
    if( icmp_hdr->echo_id != p_engine->icmp_echo_id )
    {
      // The echo ID is different than what we sent. Ignore.
      priv_retval = ANAL_PACKET_IGNORED;
      DBG_PRINT("Invalid ICMP echo ID for ECHO REPLY. Echo ID:%d\n", icmp_hdr->echo_id);
      break;
    }

    // -----------------------------------------------------------------
    // At this point we should have a valid ICMP ECHO REPLY packet that
    // belongs to us.
    // -----------------------------------------------------------------
    *echo_seq = icmp_hdr->echo_seq; // Returned echo sequence.

    // Compute the rtt.
    rtt = -_compute_tv_diff_now( (struct timeval*)icmp_hdr->echo_data );
    if( rtt > (double)p_engine->echo_timeout )
    {
      // We have received a late ICMP echo reply.
      priv_retval = ANAL_PACKET_PINGIN_LATE;
    }

    DBG_PRINT("<< %s ECHO REPLY, seq:%d, len:%d, rtt:%.3fms\n",
               ICMP_LITTERAL, icmp_hdr->echo_seq, pkt_len - ip_len, rtt );

    // Callback the receive function.
    if( p_engine->clbk_recv != NULL )
    {
      p_engine->clbk_recv( rtt );
    }
  }
  // Dummy loop end.
  while(0);

  return priv_retval;
}


// --------------------------------------------------------------------------
// _create_insert_echo_event: Private function used to create and insert an
//   echo event in the engine sorted list of echo events.
//
// Parameters:
//   p_engine: Pointer to a ICMP_ECHO_ENGINE_PARMS structure.
//   tv_sent: Pointer to a timeval structure representing the time at which
//            an ECHO REQUEST has been sent.
//   echo_seq: ECHO REQUEST sequence number.
//
// Returned value:
//   NULL on memory allocation error. The pointer to the newly created echo
//   event is returned otherwise.
//
PECHO_EVENT _create_insert_echo_event( PICMP_ECHO_ENGINE_PARMS p_engine, struct timeval* tv_sent, uint32_t echo_seq )
{
  PECHO_EVENT p_event = NULL;

  // Allocate the memory for a new echo event.
  p_event = (PECHO_EVENT)pal_malloc( sizeof(ECHO_EVENT) );
  if( p_event != NULL )
  {
    p_event->next = NULL;
    p_event->echo_seq = echo_seq;
    memcpy( &p_event->tv_timeout, tv_sent, sizeof(struct timeval) );

    // Add the default echo timeout to get time at which this echo request
    //   will timeout.
    _compute_echo_timeout( p_engine, &p_event->tv_timeout );

    // Insert the echo event in the sorted list of echo events.
    _insert_echo_event( p_engine, p_event );
  }

  return p_event;
}


// --------------------------------------------------------------------------
// _insert_echo_event: Private function used to insert an echo event in the
//   echo engine sorted list of echo events.
//
// Parameter:
//   p_engine: Pointer to a ICMP_ECHO_ENGINE_PARMS structure.
//   p_event_insert: Echo event to insert.
//
// Return value: (none)
//
void _insert_echo_event( PICMP_ECHO_ENGINE_PARMS p_engine, PECHO_EVENT p_event_insert )
{
  PECHO_EVENT * pp_event = &(p_engine->event_list);

  // Insert the event in the sorted list.
  do
  {
    // End of list; insert here.
    if( *pp_event == NULL )
    {
      *pp_event = p_event_insert;
      p_event_insert->next = NULL;
      break;
    }

    // Check if the p_event_insert is due later than the current list element.
    if( _compare_tv( &p_event_insert->tv_timeout, &(*pp_event)->tv_timeout ) > 0.0 )
    {
      // The event to insert is due later.
      // => Move to the next element in the list.
      pp_event = &((*pp_event)->next);
    }
    else
    {
      // The event to insert is due sooner.
      // => Insert before the current list element.
      p_event_insert->next = *pp_event;
      (*pp_event)->next = p_event_insert;
      break;
    }
  }
  while(1); // Endless loop.
}


// --------------------------------------------------------------------------
// _remove_free_echo_event: Private function used to remove and free an echo
//   event from the echo engine sorted list of echo events.
//
// Parameters:
//   p_engine: Pointer to a ICMP_ECHO_ENGINE_PARMS structure.
//   echo_id: The echo id of the event to remove and free.
//
// Return value:
//   ECHO_EVENT_REMOVED if the echo event was found, and removed.
//   ECHO_EVENT_NOTFOUND if the echo event could not be found.
//
iee_priv_ret_t _remove_free_echo_event( PICMP_ECHO_ENGINE_PARMS p_engine, uint32_t echo_seq )
{
  iee_priv_ret_t priv_retval = ECHO_EVENT_NOTFOUND;
  PECHO_EVENT *pp_event = &(p_engine->event_list);
  PECHO_EVENT p_event_to_free = NULL;

  while( *pp_event != NULL )
  {
    if( (*pp_event)->echo_seq == echo_seq )
    {
      // Save the pointer to the echo event to remove.
      p_event_to_free = *pp_event;

      // Fix the chained event list.
      *pp_event = (*pp_event)->next;

      // Free the echo event.
      pal_free(p_event_to_free);

      priv_retval = ECHO_EVENT_REMOVED;
      break;
    }
    pp_event = &(*pp_event)->next;
  }

  return priv_retval;
}


// --------------------------------------------------------------------------
// _free_echo_event_list: Private function used to free a list of echo event
//   list. This function is called back recursively.
//
// Parameter:
//   p_event_list: list of echo events.
//
// Return value: (none)
//
void _free_echo_event_list( PECHO_EVENT p_event )
{
  if( p_event != NULL )
  {
    // Free the next event firsthand.
    _free_echo_event_list( p_event->next );

    // Free this event.
    pal_free( p_event );
  }
}


// --------------------------------------------------------------------------
// _compute_next_send: private function used to compute the next value of
//   tv_next_send. Adds one send interval to the timeval.
//
// Parameters:
//   p_engine: Pointer to a ICMP_ECHO_ENGINE_PARMS structure.
//   tv_next_send: Pointer to timeval structure representing at what time the
//     next send should occur.
//
// Returned value: (none)
//
void _compute_next_send( PICMP_ECHO_ENGINE_PARMS p_engine, struct timeval* tv_next_send )
{
  tv_next_send->tv_sec  += p_engine->send_interval / 1000;
  tv_next_send->tv_usec += (p_engine->send_interval % 1000) * 1000;
}


// --------------------------------------------------------------------------
// _compute_echo_timeout: private function used to compute the timeout value
//   of an echo event.
//
// Parameters:
//   p_engine: Pointer to a ICMP_ECHO_ENGINE_PARMS structure.
//   tv_timeout: Pointer to timeval structure that represents the time at
//     which an ECHO REQUEST was sent.
//
// Returned value: (none)
//
void _compute_echo_timeout( PICMP_ECHO_ENGINE_PARMS p_engine, struct timeval* tv_timeout )
{
  if( p_engine->eng_mode == IEE_MODE_ACD )
  {
    // When the IEE is in mode ACD, the echo timeout is calculated in a
    //   different way.
    p_engine->echo_timeout = p_engine->send_interval * (p_engine->echo_num - p_engine->count_send);
  }

  // Normal behavior: Add the echo timeout parameter to tv_timeout.
  tv_timeout->tv_sec  += p_engine->echo_timeout / 1000;
  tv_timeout->tv_usec += (p_engine->echo_timeout % 1000) * 1000;

  // Check tv_usec overflow:
  while( tv_timeout->tv_usec > 1000000 )
  {
    tv_timeout->tv_usec -= 1000000;
    tv_timeout->tv_sec++;
  }
}


// --------------------------------------------------------------------------
// _conv_ms_to_tv: private function to convert a double value expressing
//                 milliseconds into a timeval, hence keeping the
//                 microsecond precision.
//                 If 'ms' is negative, tv is zeroed out.
//
// Parameters:
//   ms: Value to convert. This value expresses milliseconds with microsecond
//       precision.
//   tv: Will be set to represent the ms value.
//
// Return value: (none)
//
void _conv_ms_to_tv( double ms, struct timeval* tv )
{
  if( ms > 0.0 )
  {
    tv->tv_sec  = (uint32_t)ms / 1000;
    tv->tv_usec = (uint32_t)((ms - (tv->tv_sec * 1000)) * 1000.0);
  }
  else
  {
    // Zero.
    memset( tv, 0, sizeof(*tv));
  }
}


// --------------------------------------------------------------------------
// _compute_tv_diff_now: private function used to calculate the difference
//   between a timeval value and 'now'.
//
// Parameters:
//   tv_diff: Pointer to timeval structure to diff with 'now'.
//
// Return value:
//   The number of miliseconds in which tv_diff will occur. If returned value
//   is negative, it means that tv_diff is in the past; i.e.: tv_diff < tv_now
//
double _compute_tv_diff_now( struct timeval* tv_diff )
{
  struct timeval tv_now;

  pal_gettimeofday( &tv_now );
  return _compare_tv( tv_diff, &tv_now );
}


// --------------------------------------------------------------------------
// _compare_tv: Private function use to compare two timeval structure values.
//
// Parameter:
//   tv_1: Pointer to timeval structure #1
//   tv_2: Pointer to timeval structure #2
//
// Return value:
//   The number of miliseconds of difference between each value:
//   if tv_1 > tv_2, the return value will be positive.
//   if tv_1 < tv_2, the return value will be negative.
//   If they're both equal, the return value will be zero.
//
double _compare_tv( struct timeval* tv_1, struct timeval* tv_2 )
{
  return ((double)(tv_1->tv_sec  - tv_2->tv_sec)) * 1000.0 +
         ((double)(tv_1->tv_usec - tv_2->tv_usec)) / 1000.0;
}

