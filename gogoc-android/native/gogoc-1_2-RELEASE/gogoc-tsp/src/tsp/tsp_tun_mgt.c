/*
-----------------------------------------------------------------------------
 $Id: tsp_tun_mgt.c,v 1.1 2009/11/20 16:53:42 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

// Tunnel Management functions

#include "platform.h"
#include "gogoc_status.h"

#include "tsp_tun_mgt.h"
#include "tsp_client.h"       // tspCheckForStopOrWait().
#include "net_ka.h"           // Keepalive functionality.
#include "net_ka_winxp.h"     // Use old keep-alive implementation WinXP hack.
#include "tsp_lease.h"        // Tunnel lifetime functions.
#include "log.h"              // Log

#define LOOP_WAIT_MS  500

#ifdef WIN32
// Prototype.
static gogoc_status winxp_tspPerformTunnelLoop( const PTUNNEL_LOOP_CONFIG pTunLoopCfg );
#endif

// --------------------------------------------------------------------------
// Function: tspPerformTunnelLoop
//
// Performs the 3 following actions:
//   1. Handles tunnel keepalive functionnality (if configured).
//   2. Checks tunnel lifetime expiration (if any).
//   3. Checks for tunnel termination signal (Usually user-generated).
//
//   When this function returns, program should tear down tunnel and exit.
//
// Possible return values:
//   - NO_ERROR: Exiting because we're terminating tunnel (user request).
//   - KEEP_ALIVE_ERROR: Something went wrong in keepalive initialisation.
//   - KEEP_ALIVE_TIMEOUT: Keepalive timeout occured.
//   - LEASE_EXPIRED: Tunnel lifetime has ended.
//
gogoc_status tspPerformTunnelLoop( const PTUNNEL_LOOP_CONFIG pTunLoopCfg )
{
  void* p_ka_engine = NULL;   // Keepalive engine parameters.
  ka_status_t ka_status;      // Keepalive status
  ka_ret_t ka_ret;            // keepalive return code.
  uint8_t ongoing = 1;        // Flag used to know when to stop.
  long tun_expiration = 0;    // Tunnel expiration.
  gogoc_status status = STATUS_SUCCESS_INIT;

#ifdef WIN32
  if( winxp_use_old_ka == 1 )  // set by winpc/tsp_local.c
  {
    // Use old keepalive implementation for Windows XP only.
    Display( LOG_LEVEL_1, ELInfo, "tspPerformTunnelLoop",
      "Using previous keepalive implementation for Windows XP." );
    return winxp_tspPerformTunnelLoop( pTunLoopCfg );
  }
#endif

  // Compute tunnel expiration time. (IETF DSTM Draft 6.1)
  //
  if( pTunLoopCfg->tun_lifetime != 0 )
  {
    tun_expiration = tspLeaseGetExpTime( pTunLoopCfg->tun_lifetime );
  }

  // Initialize keepalive, if configured, and start the processing.
  //
  if( pTunLoopCfg->ka_interval > 0 )
  {
    // Initialize the keepalive engine.
    ka_ret = KA_init( &p_ka_engine,
                      pTunLoopCfg->ka_interval * 1000,
                      pTunLoopCfg->ka_src_addr,
                      pTunLoopCfg->ka_dst_addr,
                      pTunLoopCfg->sa_family );
    if( ka_ret != KA_SUCCESS )
    {
      return make_status(CTX_TUNNELLOOP, ERR_KEEPALIVE_ERROR);
    }

    // Start the keepalive processing.
    ka_ret = KA_start( p_ka_engine );
    if( ka_ret != KA_SUCCESS )
    {
      KA_destroy( &p_ka_engine );
      return make_status(CTX_TUNNELLOOP, ERR_KEEPALIVE_ERROR);
    }
  }


  // Start tunnel management loop.
  //
  while( status_number(status) == SUCCESS  &&  ongoing == 1 )
  {
    // Check if we've been notified to stop processing.
    if( tspCheckForStopOrWait( LOOP_WAIT_MS ) != 0 )
    {
      // We've been notified to stop.
      ongoing = 0;
    }

    // Perform a round of keepalive.
    if( pTunLoopCfg->ka_interval > 0 )
    {
      // Check if we're stopping.
      if( ongoing == 0 )
      {
        // Stop keepalive engine.
        KA_stop( p_ka_engine );
      }

      ka_status = KA_qry_status( p_ka_engine );
      switch( ka_status )
      {
      case KA_STAT_FIN_SUCCESS:
      case KA_STAT_ONGOING:
        break;

      case KA_STAT_FIN_ERROR:
      case KA_STAT_INVALID:
        status = make_status(CTX_TUNNELLOOP, ERR_KEEPALIVE_ERROR);
        KA_stop( p_ka_engine );
        break;

      case KA_STAT_FIN_TIMEOUT:
        status = make_status(CTX_TUNNELLOOP, ERR_KEEPALIVE_TIMEOUT);
        KA_stop( p_ka_engine );
        break;
      }
    }

    // Check for tunnel lease expiration.
    if( tun_expiration != 0  &&  tspLeaseCheckExp( tun_expiration ) == 1 )
    {
      status = make_status(CTX_TUNNELLOOP, ERR_TUN_LEASE_EXPIRED);
    }
  }


  // Clean-up resources allocated for keepalive functionnality.
  //
  if( pTunLoopCfg->ka_interval > 0 )
  {
    KA_destroy( &p_ka_engine );
  }


  // Exit function & return status.
  return status;
}

#ifdef WIN32
// --------------------------------------------------------------------------
// Function: tspPerformTunnelLoop
//
//   This function implements the old keepalive algorithm. It is used only
//   by Windows XP. Windows XP cannot use new implementation because its
//   firewall always drops raw socket packets.
//   This old implementation uses Windows ICMP API which works through
//   its firewall.
//
#undef LOOP_WAIT_MS
#define LOOP_WAIT_MS 100
static gogoc_status winxp_tspPerformTunnelLoop( const PTUNNEL_LOOP_CONFIG pTunLoopCfg )
{
  gogoc_status status = STATUS_SUCCESS_INIT;
  long tun_expiration = 0;
  int ret = 0;

  // Compute tunnel expiration time. (IETF DSTM Draft 6.1)
  //
  if( pTunLoopCfg->tun_lifetime != 0 )
  {
    tun_expiration = tspLeaseGetExpTime( pTunLoopCfg->tun_lifetime );
  }

  // Initialize keep-alive, if configured.
  //
  if( pTunLoopCfg->ka_interval > 0 )
  {
    ret = NetKeepaliveInit( pTunLoopCfg->ka_src_addr,
                            pTunLoopCfg->ka_dst_addr,
                            pTunLoopCfg->ka_interval,
                            pTunLoopCfg->sa_family );

    if( ret != 0 )
    {
      // Something went wrong in keep-alive initialisation.
      return make_status(CTX_TUNNELLOOP, ERR_KEEPALIVE_ERROR);
    }
  }

  // Start tunnel management loop.
  //
  while( status_number(status) == SUCCESS  &&  tspCheckForStopOrWait( LOOP_WAIT_MS ) == 0 )
  {
    // Perform a round of keep-alive.
    if( pTunLoopCfg->ka_interval > 0  &&  NetKeepaliveDo() == 2 )
    {
      status = make_status(CTX_TUNNELLOOP, ERR_KEEPALIVE_TIMEOUT);
    }

    // Check for tunnel lease expiration.
    if( tun_expiration != 0  &&  tspLeaseCheckExp( tun_expiration ) == 1 )
    {
      status = make_status(CTX_TUNNELLOOP, ERR_TUN_LEASE_EXPIRED);
    }
  }

  // Clean-up resources allocated for keep-alive functionnality.
  //
  if( pTunLoopCfg->ka_interval > 0 )
  {
    NetKeepaliveDestroy();
  }

  // Exit function & return status.
  return status;
}
#endif
