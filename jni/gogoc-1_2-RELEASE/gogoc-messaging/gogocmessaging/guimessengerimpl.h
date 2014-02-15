// **************************************************************************
// $Id: guimessengerimpl.h,v 1.1 2009/11/20 16:34:52 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   This is the gogoCLIENT GUI implementation of the messenger 
//   subsystem. The naming of the derived class as `client' indicate that 
//   the gogoCLIENT GUI is the client-side of the messaging subsystem.
//   (The GOGOC service is the server-side of the messaging subsystem.)
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_guimessengerimpl_h__
#define __gogocmessaging_guimessengerimpl_h__


#include <gogocmessaging/clientmsgsender.h>
#include <gogocmessaging/clientmsgtranslator.h>
#include <gogocmessaging/communicationsmgr.h>
#undef PostMessage       // Because of stupid windows API PostMessage method

namespace gogocmessaging
{

  // ------------------------------------------------------------------------
  class GUIMessengerImpl : public ClientMsgSender, public ClientMsgTranslator
  {
  public:
    typedef void    (*RecvStatusInfo)     ( const gogocStatusInfo* );
    typedef void    (*RecvTunnelInfo)     ( const gogocTunnelInfo* );
    typedef void    (*RecvBrokerList)     ( const gogocBrokerList* );
    typedef void    (*RecvHACCESSStatusInfo) ( const HACCESSStatusInfo* );

  public:
    // Handling functions.
    RecvStatusInfo  m_RecvStatusInfo;
    RecvTunnelInfo  m_RecvTunnelInfo;
    RecvBrokerList  m_RecvBrokerList;
    RecvHACCESSStatusInfo m_RecvHACCESSStatusInfo;
  private:
    CommunicationsManager m_CommManager;


  public:
                    GUIMessengerImpl      ( void );
    virtual         ~GUIMessengerImpl     ( void );


    // Message processing functions.
    void            EnableProcessing      ( void );
    void            DisableProcessing     ( void );


    // Overrides from the ClientMsgTranslator:
    error_t         Recv_StatusInfo       ( const gogocStatusInfo* aStatusInfo );
    error_t         Recv_TunnelInfo       ( const gogocTunnelInfo* aTunnelInfo );
    error_t         Recv_BrokerList       ( const gogocBrokerList* aBrokerList );
    error_t         Recv_HACCESSStatusInfo   ( const HACCESSStatusInfo* aHACCESSStatusInfo );

    // Overrides from the ClientMsgSender:
    void            PostMessage           ( Message* pMsg );
  };

}

#endif
