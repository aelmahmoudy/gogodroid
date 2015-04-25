/*
-----------------------------------------------------------------------------
 $Id: net_ka.c,v 1.1 2009/11/20 16:53:39 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"

#include "net_ka.h"
#include "icmp_echo_engine.h"
#include "log.h"
#include "hex_strings.h"


#define KA_ECHO_REPLY_TIMEOUT         5000  // 5 seconds timeout.
#define KA_NUM_CONSEC_TIMEOUT         3     // 3 consecutive timeouts.

#ifndef __FUNCTION__
#define __FUNCTION__                  __func__
#endif
#define LOG_MESSAGE(LVL,SEV,MSG,...)  Display(LVL, SEV, __FUNCTION__, MSG, ##__VA_ARGS__)


// --------------------------------------------------------------------------
// Keepalive engine parameters.
typedef struct __KA_ENGINE_PARMS
{
  pal_thread_t  ka_thread_id;   // Keepalive thread ID.
  ka_status_t   ka_status;      // Keepalive engine status.
  void*         p_echo_engine;  // Opaque data used by the ICMP echo engine.
} KA_ENGINE_PARMS, * PKA_ENGINE_PARMS;


// --------------------------------------------------------------------------
// KA internal private statuses
typedef enum {
  KA_PRIV_SUCCESS,              // General operation successful.

  KA_INVALID_PARMS,             // Invalid input parameter(s).
  KA_RESOURCE_STARVATION        // Resource starvation (memory alloc error).

} ka_priv_ret_t;


// --------------------------------------------------------------------------
// Local private function prototypes
ka_priv_ret_t       _create_ka_engine     ( PKA_ENGINE_PARMS *pp_engine );
ka_priv_ret_t       _destroy_ka_engine    ( PKA_ENGINE_PARMS *pp_engine );
pal_thread_ret_t PAL_THREAD_CALL _ka_start_thread( void *arg );
void                _ka_send_callback     ( void );
void                _ka_recv_callback     ( double rtt );


// --------------------------------------------------------------------------
// KA_init: Initializes the keepalive engine.
//
// Parameters:
//   pp_engine: Double opaque pointer where the keepalive engine parameters 
//     will be stored.
//   ka_send_interval: The keepalive send interval, in milliseconds.
//   ka_src_addr: Source address used when sending keepalives.
//   ka_dst_addr: Destination address where keepalives will be sent.
//   ka_af: Address family (INET or INET6) used to send keepalives.
//
// Returned value:
//   KA_SUCCESS if initialisation was successful.
//   KA_ERROR if an error occurred.
//
ka_ret_t KA_init( void ** pp_engine, uint32_t ka_send_interval, char* ka_src_addr, char* ka_dst_addr, sint32_t ka_af )
{
  PKA_ENGINE_PARMS p_ka_engine;
  iee_ret_t iee_ret;
  ka_priv_ret_t ka_priv_ret;


  // Allocate the Keepalive engine.
  ka_priv_ret = _create_ka_engine( (PKA_ENGINE_PARMS*)pp_engine );
  switch( ka_priv_ret )
  {
  case KA_PRIV_SUCCESS:
    // No error. Continue on.
    break;

  case KA_INVALID_PARMS:
    // Invalid keepalive engine pointer.
    LOG_MESSAGE( LOG_LEVEL_1, ELError, STR_KA_INIT_FAIL_CAUSE STR_KA_ERR_ALREADY_INIT );
    return KA_ERROR;

  case KA_RESOURCE_STARVATION:
    // Memory allocation error.
    LOG_MESSAGE( LOG_LEVEL_1, ELError, STR_KA_INIT_FAIL_CAUSE STR_GEN_MALLOC_ERROR );
    return KA_ERROR;
  }

  // Cast the double opaque pointer to a local valid pointer.
  p_ka_engine = (PKA_ENGINE_PARMS)*pp_engine;

  // Initialize the ICMP ECHO engine with the keepalive parameters.
  iee_ret = IEE_init( &p_ka_engine->p_echo_engine, 
                      IEE_MODE_KA, ka_send_interval, 0, 
                      KA_ECHO_REPLY_TIMEOUT, KA_NUM_CONSEC_TIMEOUT, 
                      ka_src_addr, ka_dst_addr, ka_af,
                      &_ka_send_callback, &_ka_recv_callback );
  switch( iee_ret )
  {
  case IEE_SUCCESS:
    // Initialisation successful.
    LOG_MESSAGE( LOG_LEVEL_2, ELInfo, STR_KA_INIT_INFO, ka_dst_addr, 
            ka_send_interval, KA_ECHO_REPLY_TIMEOUT, KA_NUM_CONSEC_TIMEOUT );
    break;

  case IEE_INVALID_PARMS:
    // Invalid parameters.
    LOG_MESSAGE( LOG_LEVEL_1, ELError, STR_KA_INIT_FAIL_CAUSE STR_GEN_INVALID_POINTER );
    _destroy_ka_engine( &p_ka_engine );
    return KA_ERROR;

  case IEE_RESOURCE_STARVATION:
    // Memory allocation error.
    LOG_MESSAGE( LOG_LEVEL_1, ELError, STR_KA_INIT_FAIL_CAUSE STR_GEN_MALLOC_ERROR );
    _destroy_ka_engine( &p_ka_engine );
    return KA_ERROR;

  default:
    // Should never happen.
    LOG_MESSAGE( LOG_LEVEL_1, ELError, STR_KA_INIT_FAIL_CAUSE STR_GEN_UNKNOWN_ERROR );
    _destroy_ka_engine( &p_ka_engine );
    return KA_ERROR;
  }

  return KA_SUCCESS;
}


// --------------------------------------------------------------------------
// KA_start: Start the keepalive main processing thread. Returns immediately
//   (non-blocking).
//
// Parameter:
//   p_engine: Opaque pointer to the Keepalive engine.
//
// Return values:
//   KA_SUCCESS indicates the keepalive main thread was started.
//   KA_ERROR on error
//
ka_ret_t KA_start( void * p_engine )
{
  PKA_ENGINE_PARMS p_ka_engine = (PKA_ENGINE_PARMS)p_engine;
  sint32_t ret;


  // Check KA engine pointer validity.
  if( p_ka_engine == NULL )
  {
    // Evil is at work someplace..
    LOG_MESSAGE( LOG_LEVEL_1, ELError, STR_KA_START_FAIL_CAUSE STR_GEN_INVALID_POINTER );
    return KA_ERROR;
  }

  // Start a new thread to process the keepalive messages.
  ret = pal_thread_create( &p_ka_engine->ka_thread_id, _ka_start_thread, p_ka_engine );
  if( ret != 0 )
  {
    // Error starting the keepalive main thread.
    LOG_MESSAGE( LOG_LEVEL_1, ELError, "%s%d", STR_KA_START_FAIL_CAUSE STR_KA_ERR_THREAD_START, ret );
    return KA_ERROR;
  }

  // Change the engine status.
  p_ka_engine->ka_status = KA_STAT_ONGOING;

  return KA_SUCCESS;
}


// --------------------------------------------------------------------------
// KA_stop: Stops the ICMP echo engine and wait for the main keepalive 
//   thread to finish. Retrieve the keepalive status with KA_get_status 
//   after the stop is done.
//
// Parameters:
//   p_engine: Opaque pointer to the Keepalive engine.
//
// Return value:
//   KA_SUCCESS indicates a successful stop.
//   KA_ERROR on error.
//
ka_ret_t KA_stop( void * p_engine )
{
  PKA_ENGINE_PARMS p_ka_engine = (PKA_ENGINE_PARMS)p_engine;
  iee_ret_t iee_ret;
  sint32_t ret;

  // Check echo engine opaque pointer validity.
  if( p_ka_engine == NULL  ||  p_ka_engine->p_echo_engine == NULL )
  {
    // Evil is at work someplace..
    LOG_MESSAGE( LOG_LEVEL_1, ELError, STR_KA_STOP_FAIL_CAUSE STR_GEN_INVALID_POINTER );
    return KA_ERROR;
  }

  // Issue the stop to the engine. This will asynchronously cause the KA 
  // thread to exit eventually.
  iee_ret = IEE_stop( p_ka_engine->p_echo_engine );
  if( iee_ret != IEE_SUCCESS )
  {
    // Error stopping the echo engine.
    LOG_MESSAGE( LOG_LEVEL_1, ELError, STR_KA_STOP_FAIL_CAUSE STR_KA_ERR_STOP_IEE );
    return KA_ERROR;
  }

  // Wait on the KA thread to finish.
  ret = pal_thread_join( p_ka_engine->ka_thread_id, NULL );
  if( ret != 0 )
  {
    // Error joining the keepalive thread.
    LOG_MESSAGE( LOG_LEVEL_1, ELError, "%s%d", STR_KA_STOP_FAIL_CAUSE STR_KA_ERR_THREAD_JOIN, ret );
    return KA_ERROR;
  }

  return KA_SUCCESS;
}


// --------------------------------------------------------------------------
// KA_qry_status: Retrieves the status of the keepalive engine.
//
// Parameter:
//   p_engine: Opaque pointer to the Keepalive engine.
//
// Return value:
//   The keepalive status (One of the ka_status_t enum).
//   If the pointer p_engine is invalid, KA_STAT_INVALID is returned.
//
ka_status_t KA_qry_status( void * p_engine )
{
  PKA_ENGINE_PARMS p_ka_engine = (PKA_ENGINE_PARMS)p_engine;

  // Check echo engine opaque pointer validity.
  if( p_ka_engine == NULL )
  {
    LOG_MESSAGE( LOG_LEVEL_1, ELError, STR_KA_QRYSTATUS_FAIL_CAUSE STR_GEN_INVALID_POINTER );
    return KA_STAT_INVALID;
  }

  // Return the keepalive status.
  return p_ka_engine->ka_status;
}


// --------------------------------------------------------------------------
// KA_destroy: Deallocates memory used by the ICMP echo engine.
//
// Parameters:
//   pp_engine: Double opaque pointer to the Keepalive engine.
//
// Return value:
//   KA_SUCCESS on success.
//   KA_ERROR if the destruction of the ICMP echo engine was not successful.
//
ka_ret_t KA_destroy( void ** pp_engine )
{
  PKA_ENGINE_PARMS p_ka_engine = (PKA_ENGINE_PARMS)*pp_engine;
  ka_priv_ret_t ka_priv_ret;
  iee_ret_t iee_ret;


  // Check echo engine opaque pointer validity.
  if( p_ka_engine == NULL  ||  p_ka_engine->p_echo_engine == NULL )
  {
    // Invalid pp_engine pointer.
    LOG_MESSAGE( LOG_LEVEL_1, ELError, STR_KA_DESTR_FAIL_CAUSE STR_GEN_INVALID_POINTER );
    return KA_ERROR;
  }

  // Destroy the ICMP echo engine.
  iee_ret = IEE_destroy( &p_ka_engine->p_echo_engine );
  if( iee_ret != IEE_SUCCESS )
  {
    // Failed to destroy the ICMP echo engine.
    LOG_MESSAGE(LOG_LEVEL_1, ELError, STR_KA_DESTR_FAIL_CAUSE STR_KA_ERR_DESTR_IEE );
    return KA_ERROR;
  }

  // Destroy the KA engine.
  ka_priv_ret = _destroy_ka_engine( (PKA_ENGINE_PARMS*)pp_engine );
  switch( ka_priv_ret )
  {
  case KA_PRIV_SUCCESS:
    // Success.
    break;

  case KA_INVALID_PARMS:
    // Invalid pp_engine pointer.
    LOG_MESSAGE(LOG_LEVEL_1, ELError, STR_KA_DESTR_FAIL_CAUSE STR_GEN_INVALID_POINTER );
    return KA_ERROR;

  default:
    // Should never happen.
    LOG_MESSAGE( LOG_LEVEL_1, ELError, STR_KA_DESTR_FAIL_CAUSE STR_GEN_UNKNOWN_ERROR );
    return KA_ERROR;
  }

  return KA_SUCCESS;
}


// --------------------------------------------------------------------------
// _create_ka_engine: Allocates the Keepalive engine and initializes it.
//
// Parameter:
//   pp_engine: double pointer to the keepalive engine.
//
// Returned values:
//   KA_PRIV_SUCCESS: Operation successful.
//   KA_RESOURCE_STARVATION: Memory allocation error.
//   KA_INVALID_PARMS: Invalid pp_engine pointer.
//
ka_priv_ret_t _create_ka_engine( PKA_ENGINE_PARMS* pp_engine )
{
  if( pp_engine == NULL  ||  *pp_engine != NULL )
  {
    // Invalid double pointer or pointer is not initialized to NULL.
    return KA_INVALID_PARMS;
  }

  *pp_engine = (PKA_ENGINE_PARMS)pal_malloc( sizeof(KA_ENGINE_PARMS) );
  if( *pp_engine == NULL )
  {
    // Failed to allocate memory for the keepalive engine.
    return KA_RESOURCE_STARVATION;
  }

  // Initialize the keepalive engine parameters.
  (*pp_engine)->ka_thread_id = 0;
  (*pp_engine)->ka_status = KA_STAT_INVALID;
  (*pp_engine)->p_echo_engine = NULL;


  return KA_PRIV_SUCCESS;
}


// --------------------------------------------------------------------------
// _destroy_ka_engine: Deallocates the memory used by the Keepalive engine.
//
// Parameter:
//   pp_engine: Double pointer to the keepalive engine.
//
// Returned values:
//   KA_PRIV_SUCCESS: Operation successful.
//   KA_INVALID_PARMS: Invalid pp_engine pointer.
//
ka_priv_ret_t _destroy_ka_engine( PKA_ENGINE_PARMS* pp_engine )
{
  if( pp_engine == NULL  ||  *pp_engine == NULL )
  {
    // Bad parameter: invalid double pointer or already freed.
    return KA_INVALID_PARMS;
  }

  // Free the reources allocated for the Keepalive engine.
  pal_free( *pp_engine );
  *pp_engine = NULL;

  return KA_PRIV_SUCCESS;
}


// --------------------------------------------------------------------------
// _ka_start_thread: Private worker thread function used to run the keep-
//   alive processing. Calls the IEE_process function until it returns.
//
// Parameters:
//   arg: Opaque pointer to the keepalive engine.
//
// Return value:
//   The keepalive engine final status.
//
pal_thread_ret_t PAL_THREAD_CALL _ka_start_thread( void *arg )
{
  PKA_ENGINE_PARMS p_ka_engine = (PKA_ENGINE_PARMS)arg;
  iee_ret_t iee_ret;


  // Check input pointer.
  if( p_ka_engine != NULL )
  {
    // Let the ICMP echo engine process the keepalive echo messages.
    iee_ret = IEE_process( p_ka_engine->p_echo_engine );
    switch( iee_ret )
    {
    case IEE_SUCCESS:
      // Keepalive processing stopped.
      // Most probable cause: KA_stop() was invoked.
      p_ka_engine->ka_status = KA_STAT_FIN_SUCCESS;
      LOG_MESSAGE( LOG_LEVEL_3, ELInfo, STR_KA_STOP_INFO_CAUSE STR_KA_EXPLICIT_STOP );
      break;

    case IEE_GENERAL_ECHO_TIMEOUT:
      // Keepalive timeout detected!
      p_ka_engine->ka_status = KA_STAT_FIN_TIMEOUT;
      LOG_MESSAGE( LOG_LEVEL_1, ELWarning, STR_KA_STOP_INFO_CAUSE STR_KA_GENERAL_TIMEOUT );
      break;

    case IEE_INVALID_PARMS:
      // Input error.
      p_ka_engine->ka_status = KA_STAT_FIN_ERROR;
      LOG_MESSAGE( LOG_LEVEL_1, ELError, STR_KA_STOP_INFO_CAUSE STR_GEN_INVALID_POINTER );
      break;

    case IEE_GENERAL_ECHO_ERROR:
      // Keepalive processing error.
      p_ka_engine->ka_status = KA_STAT_FIN_ERROR;
      LOG_MESSAGE( LOG_LEVEL_1, ELError, STR_KA_STOP_INFO_CAUSE STR_GEN_NETWORK_ERROR );
      break;

    default:
      // Unknown/Unhandled ERROR.
      p_ka_engine->ka_status = KA_STAT_FIN_ERROR;
      LOG_MESSAGE( LOG_LEVEL_1, ELError, STR_KA_STOP_INFO_CAUSE STR_GEN_UNKNOWN_ERROR );
      break;
    }
  }
  else
  {
    // Invalid pointer input.
    LOG_MESSAGE( LOG_LEVEL_1, ELError, STR_KA_STOP_INFO_CAUSE STR_GEN_INVALID_POINTER );
  }

  // Exit the thread.
  pal_thread_exit( 0 );
  return 0;
}


// --------------------------------------------------------------------------
// _ka_send_callback: Function called back from the ICMP echo engine upon
//   successful send of an echo request message.
//
// Parameter: (none)
//
// Return value: (none)
//
void _ka_send_callback( void )
{
  LOG_MESSAGE( LOG_LEVEL_3, ELInfo, STR_KA_SEND_INFO );
}


// --------------------------------------------------------------------------
// _ka_recv_callback: Function called back from the ICMP echo engine upon
//   successful reception of an echo reply message.
//
// Parameter:
//   rtt: Keepalive roundtrip time.
//
// Return value: (none)
//
void _ka_recv_callback( double rtt )
{
  LOG_MESSAGE( LOG_LEVEL_3, ELInfo, STR_KA_RECV_INFO, rtt );
}
