// **************************************************************************
// $Id: clientmsgdataretriever.h,v 1.1 2009/11/20 16:34:51 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Defines prototypes used by the messenger to retrieve information on
//     - status, tunnel and brokers.
//   Defines a way of freeing the information allocated in the retrievers.
//
//   These functions need to be implemented in the gogoCLIENT.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_clientsgdataretriever_h__
#define __gogocmessaging_clientsgdataretriever_h__


#include <gogocmessaging/gogocmsgdata.h>
#include <gogocmessaging/haccessmsgdata.h>
#include <gogocmessaging/gogocuistrings.h>      // error_t definition & codes.


#ifdef __cplusplus
extern "C" {
#endif

error_t   RetrieveStatusInfo    ( gogocStatusInfo** ppStatusInfo );
error_t   RetrieveTunnelInfo    ( gogocTunnelInfo** ppTunnelInfo );
error_t   RetrieveBrokerList    ( gogocBrokerList** ppBrokerList );
error_t   RetrieveHACCESSStatusInfo( HACCESSStatusInfo** ppHACCESSStatusInfo );

void      FreeStatusInfo        ( gogocStatusInfo** ppStatusInfo );
void      FreeTunnelInfo        ( gogocTunnelInfo** ppTunnelInfo );
void      FreeBrokerList        ( gogocBrokerList** ppBrokerList );
void      FreeHACCESSStatusInfo    ( HACCESSStatusInfo** ppHACCESSStatusInfo );

#ifdef __cplusplus
}
#endif

#endif
