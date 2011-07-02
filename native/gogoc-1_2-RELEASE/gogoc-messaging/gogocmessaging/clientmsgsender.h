// **************************************************************************
// $Id: clientmsgsender.h,v 1.1 2009/11/20 16:34:51 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Defines the different messages the gogoCLIENT GUI can send.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_clientmsgsender_h__
#define __gogocmessaging_clientmsgsender_h__


#include <gogocmessaging/messagesender.h>
#include <gogocmessaging/haccessmsgdata.h>
#include <gogocmessaging/gogocuistrings.h>
#undef PostMessage


namespace gogocmessaging
{
  // ------------------------------------------------------------------------
  class ClientMsgSender
  {
  protected:
    // Construction / destruction.
                    ClientMsgSender       ( void );
  public:
    virtual         ~ClientMsgSender      ( void );

  public:
    void            Send_StatusInfoRequest( void );
    void            Send_TunnelInfoRequest( void );
    void            Send_BrokerListRequest( void );
    void            Send_HACCESSConfigInfo   ( const HACCESSConfigInfo* aHACCESSCfgInfo );
    void            Send_HACCESSStatusInfoRequest( void );

  protected:
    virtual void    PostMessage           ( Message* pMsg )=0;
  };

}

#endif
