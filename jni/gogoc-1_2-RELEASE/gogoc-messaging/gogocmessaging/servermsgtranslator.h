// **************************************************************************
// $Id: servermsgtranslator.h,v 1.1 2009/11/20 16:34:54 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
//
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Defines the message processing of gogoCLIENT messages destined
//   for the Client (which is the server-side of the communication).
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_servermsgtranslator_h__
#define __gogocmessaging_servermsgtranslator_h__


#include <gogocmessaging/messageprocessor.h>
#include <gogocmessaging/gogocuistrings.h>
#include <gogocmessaging/haccessmsgdata.h>


namespace gogocmessaging
{
  // ------------------------------------------------------------------------
  class ServerMsgTranslator: public MessageProcessor
  {
  protected:
    // Construction / destruction.
                    ServerMsgTranslator   ( void );
  public:
    virtual         ~ServerMsgTranslator  ( void );

  protected:
    // Override from MessageProcessor.
    error_t         ProcessMessage        ( Message* pMsg );

    // To be implemented by derived classes.
    virtual error_t Recv_StatusInfoRequest( void )=0;
    virtual error_t Recv_TunnelInfoRequest( void )=0;
    virtual error_t Recv_BrokerListRequest( void )=0;
    virtual error_t Recv_HACCESSConfigInfo   ( const HACCESSConfigInfo* aHACCESSConfigInfo )=0;
    virtual error_t Recv_HACCESSStatusInfoRequest( void )=0;

  private:
    // Message data translators.
    error_t         TranslateStatusInfoReq( uint8_t* pData, const uint16_t nDataLen );
    error_t         TranslateTunnelInfoReq( uint8_t* pData, const uint16_t nDataLen );
    error_t         TranslateBrokerListReq( uint8_t* pData, const uint16_t nDataLen );
    error_t         TranslateHACCESSConfigInfo( uint8_t* pData, const uint16_t nDataLen );
    error_t         TranslateHACCESSStatusInfoReq( uint8_t* pData, const uint16_t nDataLen );
  };

}

#endif
