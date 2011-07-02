// **************************************************************************
// $Id: clientmsgsender.cc,v 1.1 2009/11/20 16:34:55 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
//
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Implementation of the ClientMsgSender class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gogocmessaging/clientmsgsender.h>
#include <assert.h>


namespace gogocmessaging
{
// --------------------------------------------------------------------------
// Function : ClientMsgSender constructor
//
// Description:
//   Will initialize a new ClientMsgSender object.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
ClientMsgSender::ClientMsgSender( void )
{
}

// --------------------------------------------------------------------------
// Function : ClientMsgSender destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
ClientMsgSender::~ClientMsgSender( void )
{
}


// --------------------------------------------------------------------------
// Function : Send_StatusRequest
//
// Description:
//   Will post a message for a status info request.
//
// Arguments: (none)
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void ClientMsgSender::Send_StatusInfoRequest( void )
{
  Message* pMsg;


  // No need to send data in requests messages.
  pMsg = Message::CreateMessage( MESSAGEID_REQUEST_STATUSINFO, 0, NULL );
  assert( pMsg != NULL );


  // Post the message.
  PostMessage( pMsg );
}


// --------------------------------------------------------------------------
// Function : Send_StatusRequest
//
// Description:
//   Will post a message for a tunnel info request.
//
// Arguments: (none)
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void ClientMsgSender::Send_TunnelInfoRequest( void )
{
  Message* pMsg;


  // No need to send data in requests messages.
  pMsg = Message::CreateMessage( MESSAGEID_REQUEST_TUNNELINFO, 0, NULL );
  assert( pMsg != NULL );


  // Post the message.
  PostMessage( pMsg );
}


// --------------------------------------------------------------------------
// Function : Send_StatusRequest
//
// Description:
//   Will post a message for a broker list request.
//
// Arguments: (none)
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void ClientMsgSender::Send_BrokerListRequest( void )
{
  Message* pMsg;


  // No need to send data in requests messages.
  pMsg = Message::CreateMessage( MESSAGEID_REQUEST_BROKERLIST, 0, NULL );
  assert( pMsg != NULL );


  // Post the message.
  PostMessage( pMsg );
}


// --------------------------------------------------------------------------
// Function : Send_StatusRequest
//
// Description:
//   Will post a message for a broker list request.
//
// Arguments: (none)
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void ClientMsgSender::Send_HACCESSConfigInfo( const HACCESSConfigInfo* aHACCESSCfgInfo )
{
  Message* pMsg;
  uint8_t pData[MSG_MAX_USERDATA];
  uint32_t nDataLen = 0;

  assert( aHACCESSCfgInfo != NULL );


  // ----------------------------------------------------
  // Insert HACCESS Configuration Info data in the message.
  // ----------------------------------------------------

  // Insert WWW document root.
  if( aHACCESSCfgInfo->haccess_doc_root ) {
    memcpy( pData + nDataLen, aHACCESSCfgInfo->haccess_doc_root, strlen(aHACCESSCfgInfo->haccess_doc_root) + 1 );
    nDataLen += pal_strlen(aHACCESSCfgInfo->haccess_doc_root) + 1;
  }
  else {
    memset( pData + nDataLen, 0x00, 1 );
    ++nDataLen;
  }

  // Append proxy enabled.
  memcpy( pData + nDataLen, (void*)&(aHACCESSCfgInfo->haccess_proxy_enabled), sizeof(aHACCESSCfgInfo->haccess_proxy_enabled) );
  nDataLen += sizeof(aHACCESSCfgInfo->haccess_proxy_enabled);

  // Append web enabled.
  memcpy( pData + nDataLen, (void*)&(aHACCESSCfgInfo->haccess_web_enabled), sizeof(aHACCESSCfgInfo->haccess_web_enabled) );
  nDataLen += sizeof(aHACCESSCfgInfo->haccess_web_enabled);

  // Append mappings changed.
  memcpy( pData + nDataLen, (void*)&(aHACCESSCfgInfo->haccess_devmap_changed), sizeof(aHACCESSCfgInfo->haccess_devmap_changed) );
  nDataLen += sizeof(aHACCESSCfgInfo->haccess_devmap_changed);


  assert( nDataLen <= MSG_MAX_USERDATA );       // Buffer overflow has occured.


  // Create Message.
  pMsg = Message::CreateMessage( MESSAGEID_HACCESSCONFIGINFO, nDataLen, pData );
  assert( pMsg != NULL );


  // Post the message.
  PostMessage( pMsg );
}


// --------------------------------------------------------------------------
// Function : Send_HACCESSStatusInfoRequest
//
// Description:
//   Will post a message for a HACCESS Status Info request.
//
// Arguments: (none)
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void ClientMsgSender::Send_HACCESSStatusInfoRequest( void )
{
  Message* pMsg;


  // No need to send data in requests messages.
  pMsg = Message::CreateMessage( MESSAGEID_REQUEST_HACCESSSTATUSINFO, 0, NULL );
  assert( pMsg != NULL );


  // Post the message.
  PostMessage( pMsg );
}

} // namespace
