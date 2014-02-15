// **************************************************************************
// $Id: servermsgsender.cc,v 1.1 2009/11/20 16:34:56 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
//
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Implementation of the ServerMsgSender class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gogocmessaging/servermsgsender.h>
#include <assert.h>


namespace gogocmessaging
{
// --------------------------------------------------------------------------
// Function : ServerMsgSender constructor
//
// Description:
//   Will initialize a new ServerMsgSender object.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
ServerMsgSender::ServerMsgSender( void )
{
}

// --------------------------------------------------------------------------
// Function : ServerMsgSender destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
ServerMsgSender::~ServerMsgSender( void )
{
}


// --------------------------------------------------------------------------
// Function : Send_StatusInfo
//
// Description:
//   Will send information on the status of the gogoCLIENT.
//   A message is created with the information and posted to the send queue.
//
// Arguments:
//   aStatusInfo: gogocStatusInfo* [in], The current state of the gogoCLIENT
//                client.
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void ServerMsgSender::Send_StatusInfo( const gogocStatusInfo* aStatusInfo )
{
  Message* pMsg;
  uint8_t pData[MSG_MAX_USERDATA];
  unsigned int nDataLen = 0;


  assert( aStatusInfo != NULL );


  // Write client status to data buffer.
  memcpy( pData, (void*)&(aStatusInfo->eStatus), sizeof(gogocCliStatus) );
  nDataLen += sizeof(gogocCliStatus);

  // Append status code to nStatus.
  memcpy( pData + nDataLen, (void*)&(aStatusInfo->nStatus), sizeof(aStatusInfo->nStatus) );
  nDataLen += sizeof(aStatusInfo->nStatus);


  assert( nDataLen <= MSG_MAX_USERDATA );       // Buffer overflow has occured.


  // Create Message
  pMsg = Message::CreateMessage( MESSAGEID_STATUSINFO, nDataLen, pData );
  assert( pMsg != NULL );


  // Post the message.
  PostMessage( pMsg );
}


// --------------------------------------------------------------------------
// Function : Send_TunnelInfo
//
// Description:
//   Will send information on the established tunnel of the gogoCLIENT.
//   A message is created with the information and posted to the send queue.
//
// Arguments: (none)
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void ServerMsgSender::Send_TunnelInfo( const gogocTunnelInfo* aTunnelInfo )
{
  Message* pMsg;
  uint8_t pData[MSG_MAX_USERDATA];
  uint16_t nDataLen = 0;


  assert( aTunnelInfo != NULL );


  // Append broker name to data buffer.
  if( aTunnelInfo->szBrokerName ) {
    memcpy( pData + nDataLen, aTunnelInfo->szBrokerName, strlen(aTunnelInfo->szBrokerName) + 1 );
    nDataLen += (uint16_t)strlen(aTunnelInfo->szBrokerName) + 1;
  }
  else {
    memset( pData + nDataLen, 0x00, 1 );
    ++nDataLen;
  }

  // Append tunnel type to data buffer.
  memcpy( pData + nDataLen, (void*)&(aTunnelInfo->eTunnelType), sizeof(gogocTunnelType) );
  nDataLen += sizeof(gogocTunnelType);

  // Append Local tunnel endpoint IPv4 address to data buffer.
  if( aTunnelInfo->szIPV4AddrLocalEndpoint ) {
    memcpy( pData + nDataLen, aTunnelInfo->szIPV4AddrLocalEndpoint, strlen(aTunnelInfo->szIPV4AddrLocalEndpoint) + 1 );
    nDataLen += (uint16_t)strlen(aTunnelInfo->szIPV4AddrLocalEndpoint) + 1;
  }
  else {
    memset( pData + nDataLen, 0x00, 1 );
    ++nDataLen;
  }

  // Append Local tunnel endpoint IPv6 address to data buffer.
  if( aTunnelInfo->szIPV6AddrLocalEndpoint ) {
    memcpy( pData + nDataLen, aTunnelInfo->szIPV6AddrLocalEndpoint, strlen(aTunnelInfo->szIPV6AddrLocalEndpoint) + 1 );
    nDataLen += (uint16_t)strlen(aTunnelInfo->szIPV6AddrLocalEndpoint) + 1;
  }
  else {
    memset( pData + nDataLen, 0x00, 1 );
    ++nDataLen;
  }

  // Append Remote tunnel endpoint IPv4 address to data buffer.
  if( aTunnelInfo->szIPV4AddrRemoteEndpoint ) {
    memcpy( pData + nDataLen, aTunnelInfo->szIPV4AddrRemoteEndpoint, strlen(aTunnelInfo->szIPV4AddrRemoteEndpoint) + 1 );
    nDataLen += (uint16_t)strlen(aTunnelInfo->szIPV4AddrRemoteEndpoint) + 1;
  }
  else {
    memset( pData + nDataLen, 0x00, 1 );
    ++nDataLen;
  }

  // Append Remote tunnel endpoint IPv6 address to data buffer.
  if( aTunnelInfo->szIPV6AddrRemoteEndpoint ) {
    memcpy( pData + nDataLen, aTunnelInfo->szIPV6AddrRemoteEndpoint, strlen(aTunnelInfo->szIPV6AddrRemoteEndpoint) + 1 );
    nDataLen += (uint16_t)strlen(aTunnelInfo->szIPV6AddrRemoteEndpoint) + 1;
  }
  else {
    memset( pData + nDataLen, 0x00, 1 );
    ++nDataLen;
  }

  // Append The delegated prefix to data buffer.
  if( aTunnelInfo->szDelegatedPrefix ) {
    memcpy( pData + nDataLen, aTunnelInfo->szDelegatedPrefix, strlen(aTunnelInfo->szDelegatedPrefix) + 1 );
    nDataLen += (uint16_t)strlen(aTunnelInfo->szDelegatedPrefix) + 1;
  }
  else {
    memset( pData + nDataLen, 0x00, 1 );
    ++nDataLen;
  }

  // Append The delegated user domain to data buffer.
  if( aTunnelInfo->szUserDomain ) {
    memcpy( pData + nDataLen, aTunnelInfo->szUserDomain, strlen(aTunnelInfo->szUserDomain) + 1 );
    nDataLen += (uint16_t)strlen(aTunnelInfo->szUserDomain) + 1;
  }
  else {
    memset( pData + nDataLen, 0x00, 1 );
    ++nDataLen;
  }

  // Append tunnel uptime to data buffer.
  memcpy( pData + nDataLen, (void*)&(aTunnelInfo->tunnelUpTime), sizeof(time_t) );
  nDataLen += sizeof(time_t);

  assert( nDataLen <= MSG_MAX_USERDATA );       // Buffer overflow has occured.


  // Create Message.
  pMsg = Message::CreateMessage( MESSAGEID_TUNNELINFO, nDataLen, pData );
  assert( pMsg != NULL );


  // Post the message.
  PostMessage( pMsg );
}


// --------------------------------------------------------------------------
// Function : Send_BrokerList
//
// Description:
//   Will send a list of brokers.
//   A message is created with the information and posted to the send queue.
//
// Arguments:
//   aBrokerList: gogocBrokerList* [IN], The list of broker names.
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void ServerMsgSender::Send_BrokerList( const gogocBrokerList* aBrokerList )
{
  Message* pMsg;
  uint8_t pData[MSG_MAX_USERDATA];
  gogocBrokerList* list = (gogocBrokerList*)aBrokerList;
  unsigned int nDataLen = 0;


  assert( aBrokerList != NULL );


  // Loop until the list is empty.
  do
  {
    // Append broker name to data buffer.
    if( list->szAddress ) {
      memcpy( pData + nDataLen, list->szAddress, strlen(list->szAddress) + 1 );
      nDataLen += (uint16_t)strlen(list->szAddress) + 1;
    }
    else {
      memset( pData + nDataLen, 0x00, 1 );
      ++nDataLen;
    }

    // Append distance
    memcpy( pData + nDataLen, (void*)&(list->nDistance), sizeof(int) );
    nDataLen += sizeof(int);


    assert( nDataLen <= MSG_MAX_USERDATA );     // Buffer overflow occured.

  } while( (list = list->next) != NULL );


  // Create Message.
  pMsg = Message::CreateMessage( MESSAGEID_BROKERLIST, nDataLen, pData );
  assert( pMsg != NULL );


  // Post the message.
  PostMessage( pMsg );
}



// --------------------------------------------------------------------------
// Function : Send_HACCESSStatusInfo
//
// Description:
//   Will send the HACCESS Status Info.
//   A message is created with the information and posted to the send queue.
//
// Arguments:
//   aHACCESSStatusInfo: HACCESSStatusInfo* [IN], The HACCESS Status info.
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void ServerMsgSender::Send_HACCESSStatusInfo( const HACCESSStatusInfo* aHACCESSStatusInfo )
{
  Message* pMsg;
  uint8_t pData[MSG_MAX_USERDATA];
  PMAPPING_STATUS list = aHACCESSStatusInfo->haccess_devmap_statuses;
  unsigned int nDataLen = 0;


  assert( aHACCESSStatusInfo != NULL );


  // Insert HACCESS features statuses (web, proxy, DMM).
  // Append HACCESS proxying status
  memcpy( pData + nDataLen, (void*)&(aHACCESSStatusInfo->haccess_proxy_status), sizeof(HACCESSFeatStts) );
  nDataLen += sizeof(HACCESSFeatStts);

  // Append HACCESS web service status
  memcpy( pData + nDataLen, (void*)&(aHACCESSStatusInfo->haccess_web_status), sizeof(HACCESSFeatStts) );
  nDataLen += sizeof(HACCESSFeatStts);

  // Append Device Mapping Module status
  memcpy( pData + nDataLen, (void*)&(aHACCESSStatusInfo->haccess_devmapmod_status), sizeof(HACCESSFeatStts) );
  nDataLen += sizeof(HACCESSFeatStts);


  // Append the device mapping statuses.
  if( list != NULL )
  {
    do
    {
      // Append device name to data buffer.
      if( list->device_name ) {
        memcpy( pData + nDataLen, list->device_name, strlen(list->device_name) + 1 );
        nDataLen += (uint16_t)strlen(list->device_name) + 1;
      }
      else {
        memset( pData + nDataLen, 0x00, 1 );
        ++nDataLen;
      }

      // Append mapping status
      memcpy( pData + nDataLen, (void*)&(list->mapping_status), sizeof(HACCESSDevMapStts) );
      nDataLen += sizeof(HACCESSDevMapStts);

      assert( nDataLen <= MSG_MAX_USERDATA );     // Buffer overflow occured.

    } while( (list = list->next) != NULL );
  }


  // Create Message.
  pMsg = Message::CreateMessage( MESSAGEID_HACCESSSTATUSINFO, nDataLen, pData );
  assert( pMsg != NULL );


  // Post the message.
  PostMessage( pMsg );
}

} // namespace
