// **************************************************************************
// $Id: clientmsgtranslator.h,v 1.1 2009/11/20 16:34:52 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
//
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Defines the message processing of gogoCLIENT messages destined
//   for the GUI (which is the client-side).
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_clientmsgtranslator_h__
#define __gogocmessaging_clientmsgtranslator_h__


#include <gogocmessaging/messageprocessor.h>
#include <gogocmessaging/gogocuistrings.h>
#include <gogocmessaging/gogocmsgdata.h>
#include <gogocmessaging/haccessmsgdata.h>


namespace gogocmessaging
{
  // ------------------------------------------------------------------------
  class ClientMsgTranslator: public MessageProcessor
  {
  protected:
    // Construction / destruction.
                    ClientMsgTranslator   ( void );
  public:
    virtual         ~ClientMsgTranslator  ( void );

  protected:
    // Override from MessageProcessor.
    error_t         ProcessMessage        ( Message* pMsg );

    // To be implemented by derived classes.
    virtual error_t Recv_StatusInfo       ( const gogocStatusInfo* aStatusInfo )=0;
    virtual error_t Recv_TunnelInfo       ( const gogocTunnelInfo* aTunnelInfo )=0;
    virtual error_t Recv_BrokerList       ( const gogocBrokerList* aBrokerList )=0;
    virtual error_t Recv_HACCESSStatusInfo   ( const HACCESSStatusInfo* aHACCESSStatusInfo )=0;

  private:
    // Message data translators.
    error_t         TranslateStatusInfo   ( uint8_t* pData, const uint16_t nDataLen );
    error_t         TranslateTunnelInfo   ( uint8_t* pData, const uint16_t nDataLen );
    error_t         TranslateBrokerList   ( uint8_t* pData, const uint16_t nDataLen );
    error_t         TranslateHACCESSStatusInfo( uint8_t* pData, const uint16_t nDataLen );
  };

}

#endif
