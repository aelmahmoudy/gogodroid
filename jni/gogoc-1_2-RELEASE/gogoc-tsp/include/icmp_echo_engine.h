/*
-----------------------------------------------------------------------------
 $Id: icmp_echo_engine.h,v 1.1 2009/11/20 16:53:14 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2008 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.

  This engine is used for the following two use cases (see ICMP Echo Engine
  modes enumeration):
    1. By the Keepalive(KA) engine.
    2. By the Automatic Connectivity Detection(ACD) mechanism.

  The return value of IEE_destroy() depends in which use case it was called.
  For KA, the return values can be:
    - IEE_INVALID_PARMS        : Means an invalid pp_config was passed.
    - IEE_GENERAL_ECHO_TIMEOUT : Means keepalive timeout.
    - IEE_SUCCESS              : Means execution ended. (probably gogoc exiting)
    - IEE_GENERAL_ECHO_ERROR   : Means a fatal  error occured.

  For ACD, the return values can be:
    - IEE_INVALID_PARMS        : Means an invalid pp_config was passed.
    - IEE_GENERAL_ECHO_TIMEOUT : Means all echo messages timed out.
    - IEE_CONNECTIVITY_ASSESSED: Means a reply was received.
    - IEE_GENERAL_ECHO_ERROR   : Means a fatal error occured.

-----------------------------------------------------------------------------
*/

#ifndef _ICMP_ECHO_ENGINE_H_
#define _ICMP_ECHO_ENGINE_H_


// ICMP Echo Engine public return codes.
typedef enum {
  IEE_SUCCESS = 0,            // Returned by any IEE function on success.
  IEE_INVALID_PARMS,          // Returned by any IEE function when invalid input parameter(s).
  IEE_RESOURCE_STARVATION,    // Returned by IEE_init when failed to acquire system resource (memory).
  IEE_CONNECTIVITY_ASSESSED,  // Returned by IEE_process for ACD only.
  IEE_GENERAL_ECHO_TIMEOUT,   // Returned by IEE_process for ACD and KA.
  IEE_GENERAL_ECHO_ERROR      // Returned by IEE_process when ICMP echo fatal error.
} iee_ret_t;

// ICMP Echo Engine modes. Must hold within 2 bits(because defined as such).
typedef enum {
  IEE_MODE_OTHER=0,           // Other use case, such as testing.
  IEE_MODE_KA=1,              // Keepalive use case.
  IEE_MODE_ACD=2              // Automatic Connectivity Detection use case.
} iee_mode_t;

typedef void        (*iee_send_clbk)      ( void );
typedef void        (*iee_recv_clbk)      ( double rtt );

// Public function prototypes.
iee_ret_t           IEE_init              ( void** pp_config,
                                            iee_mode_t eng_mode,
                                            uint32_t send_interval,
                                            uint32_t echo_num,
                                            uint32_t echo_timeout,
                                            uint8_t echo_timeout_threshold,
                                            char* src, char* dst, sint32_t af,
                                            iee_send_clbk send_clbk,
                                            iee_recv_clbk recv_clbk );

iee_ret_t           IEE_destroy           ( void** pp_config );

iee_ret_t           IEE_process           ( void* p_config );

// Should be called from an alternate thread to be of any value:
iee_ret_t           IEE_stop              ( void* p_config );

#endif
