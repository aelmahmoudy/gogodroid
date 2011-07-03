// **************************************************************************
// $Id: servermsgsender.h,v 1.1 2009/11/20 16:34:54 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Defines the different messages the gogoCLIENT can send.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_servermsgsender_h__
#define __gogocmessaging_servermsgsender_h__


#include <gogocmessaging/messagesender.h>
#include <gogocmessaging/gogocuistrings.h>
#include <gogocmessaging/gogocmsgdata.h>
#include <gogocmessaging/haccessmsgdata.h>
#undef PostMessage


namespace gogocmessaging
{
  // ------------------------------------------------------------------------
  class ServerMsgSender
  {
  protected:
    // Construction / destruction.
                    ServerMsgSender       ( void );
  public:
    virtual         ~ServerMsgSender      ( void );

  public:
    void            Send_StatusInfo       ( const gogocStatusInfo* aStatusInfo );
    void            Send_TunnelInfo       ( const gogocTunnelInfo* aTunnelInfo );
    void            Send_BrokerList       ( const gogocBrokerList* aBrokerList );
    void            Send_HACCESSStatusInfo   ( const HACCESSStatusInfo* aHACCESSStatusInfo );

  protected:
    virtual void    PostMessage           ( Message* pMsg )=0;
  };

}

#endif
