// **************************************************************************
// $Id: clientmessengerimpl.h,v 1.1 2009/11/20 16:34:51 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   This is the gogoCLIENT implementation of the messenger subsystem.
//   The naming of the derived class as `server' indicate that the gogoCLIENT
//   only means that it is the server-side of the messaging subsystem.
//   (The GUI is the client-side of the messaging subsystem.)
//
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_clientmessengerimpl_h__
#define __gogocmessaging_clientmessengerimpl_h__


#include <gogocmessaging/servermsgsender.h>
#include <gogocmessaging/servermsgtranslator.h>
#include <gogocmessaging/communicationsmgr.h>
#undef PostMessage


namespace gogocmessaging
{

  // ------------------------------------------------------------------------
  class ClientMessengerImpl : public ServerMsgSender, public ServerMsgTranslator
  {
  private:
    CommunicationsManager m_CommManager;

  public:
                    ClientMessengerImpl   ( void );
    virtual         ~ClientMessengerImpl  ( void );

    // Waits until communication manager is ready.
    bool            WaitReady             ( unsigned long ulWaitms=750 );

    // Overrides from the ServerMsgTranslator:
    error_t         Recv_StatusInfoRequest( void );
    error_t         Recv_TunnelInfoRequest( void );
    error_t         Recv_BrokerListRequest( void );
    error_t         Recv_HACCESSConfigInfo   ( const HACCESSConfigInfo* aHACCESSConfigInfo );
    error_t         Recv_HACCESSStatusInfoRequest( void );

    // Overrides from the ServerMsgSender:
    void            PostMessage           ( Message* pMsg );
  };

}

#endif
