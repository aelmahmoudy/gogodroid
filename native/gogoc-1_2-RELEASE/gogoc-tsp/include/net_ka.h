/*
-----------------------------------------------------------------------------
 $Id: net_ka.h,v 1.1 2009/11/20 16:53:15 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.

  This is the definition of the keepalive(KA) feature.

-----------------------------------------------------------------------------
*/

#ifndef _NET_KA_H_
#define _NET_KA_H_


// Keepalive public return codes.
typedef enum {
  KA_ERROR,             // Keepalive operation failed.
  KA_SUCCESS            // Keepalive operation successful.
} ka_ret_t;

// Keepalive public statuses.
typedef enum {
  KA_STAT_INVALID,      // Invalid status. This status indicates an error.
  KA_STAT_ONGOING,      // Keepalive processing currently ongoing.
  KA_STAT_FIN_SUCCESS,  // Keepalive processing finished successfully
  KA_STAT_FIN_TIMEOUT,  // Keepalive timeout has been detected.
  KA_STAT_FIN_ERROR     // Keepalive processing finished with errors
} ka_status_t;


// Keepalive public function prototypes.
ka_ret_t            KA_init               ( void ** pp_engine,
                                            uint32_t ka_send_interval,
                                            char* ka_src_addr,
                                            char* ka_dst_addr,
                                            sint32_t ka_af );

ka_ret_t            KA_start              ( void * p_engine );

ka_status_t         KA_qry_status         ( void * p_engine );

ka_ret_t            KA_stop               ( void * p_engine );

ka_ret_t            KA_destroy            ( void ** pp_engine );

#endif
