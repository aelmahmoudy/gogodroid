// **************************************************************************
// $Id: clientmsgnotifier.h,v 1.1 2009/11/20 16:34:51 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Defines prototypes used by the messenger to notify the gogoCLIENT 
//   that a message has arrived and requires processing.
//
//   These functions need to be implemented in the gogoCLIENT.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_clientmsgnotifier_h__
#define __gogocmessaging_clientmsgnotifier_h__


#include <gogocmessaging/haccessmsgdata.h>
#include <gogocmessaging/gogocuistrings.h>      // error_t definition & codes.


#ifdef __cplusplus
extern "C" {
#endif


error_t   NotifyhaccessConfigInfo  ( const HACCESSConfigInfo* aHACCESSConfigInfo );


#ifdef __cplusplus
}
#endif

#endif
