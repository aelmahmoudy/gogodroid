// **************************************************************************
// $Id: gogoc_c_wrapper.h,v 1.1 2009/11/20 16:34:52 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Provides C access to the gogoCLIENT messenger subsystem.
//   The C functionnality here is limited to the gogoCLIENT (not the 
//   GUI).
//
// Author: Charles Nepveu
//
// Creation Date: December 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_gogoc_c_wrapper_h__
#define __gogocmessaging_gogoc_c_wrapper_h__


#include <gogocmessaging/gogocuistrings.h>      // error_t definition & codes.
#include <gogocmessaging/gogocmsgdata.h>        // messaging data.
#include <gogocmessaging/haccessmsgdata.h>        // HACCESS messaging data.


#ifdef __cplusplus
extern "C" {
#endif

// Initialization of the underlying C++ object.
error_t             initialize_messaging  ( void );
error_t             uninitialize_messaging( void );

// Send functions. They don't take arguments because they'll be using
// the clientmsgdataretriever functions (See clientmsgdataretriever.h).
error_t             send_status_info      ( void );
error_t             send_tunnel_info      ( void );
error_t             send_broker_list      ( void );
error_t             send_haccess_status_info ( void );


// Will be declared in: tsp_client.c
extern gogocStatusInfo gStatusInfo;
extern gogocTunnelInfo gTunnelInfo;
extern HACCESSStatusInfo gHACCESSStatusInfo;


#ifdef __cplusplus
}
#endif

#endif
