// **************************************************************************
// $Id: clientmsgtranslator.cc,v 1.1 2009/11/20 16:34:55 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
//
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Implementation of the ClientMsgTranslator class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <pal.h>
#include <gogocmessaging/clientmsgtranslator.h>
#include <assert.h>


namespace gogocmessaging
{
// --------------------------------------------------------------------------
// Function : ClientMsgTranslator constructor
//
// Description:
//   Will initialize a new ClientMsgTranslator object.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
ClientMsgTranslator::ClientMsgTranslator( void ) :
  MessageProcessor()
{
  // Enable message processing.
  MessageProcessor::m_eProcessorState = STATE_ENABLED;
}


// --------------------------------------------------------------------------
// Function : gogocMsgClientReceiver destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
ClientMsgTranslator::~ClientMsgTranslator( void )
{
}


// --------------------------------------------------------------------------
// Function : ProcessMessage
//
// Description:
//   Will verify the message type and call the proper translator.
//   NOTE: Try not to do any lengthy operations here, because we're executing
//         in the receiver thread.
//
// Arguments:
//   pMsg: Message* [IN], The message to process.
//
// Return values:
//   GOGOCM_UIS__NOERROR: Successful operation.
//   GOGOCM_UIS_MESSAGENOTIMPL: Message not implemented.
//   GOGOCM_UIS_MSGPROCDISABLED: Message processing is disabled.
//   <any other error message>
//
// --------------------------------------------------------------------------
error_t ClientMsgTranslator::ProcessMessage( Message* pMsg )
{
  error_t retCode;
  assert( pMsg );


  // Process messages only if the message processor is enabled.
  if( m_eProcessorState == STATE_ENABLED )
  {
    // -------------------------------------
    // Verify what kind of message this is.
    // -------------------------------------
    switch( pMsg->msg.header._msgid )
    {
    case MESSAGEID_STATUSINFO:
      retCode = TranslateStatusInfo( pMsg->msg._data, pMsg->msg.header._datalen );
      break;

    case MESSAGEID_TUNNELINFO:
      retCode = TranslateTunnelInfo( pMsg->msg._data, pMsg->msg.header._datalen );
      break;

    case MESSAGEID_BROKERLIST:
      retCode = TranslateBrokerList( pMsg->msg._data, pMsg->msg.header._datalen );
      break;

    case MESSAGEID_HACCESSSTATUSINFO:
      retCode = TranslateHACCESSStatusInfo( pMsg->msg._data, pMsg->msg.header._datalen );
      break;

    default:
      retCode = GOGOCM_UIS_MESSAGENOTIMPL; // Unknown / invalid message.
      break;
    }
  }
  else
    retCode = GOGOCM_UIS_MSGPROCDISABLED;

  // Return completion status.
  return retCode;
}


// --------------------------------------------------------------------------
// Function : TranslateStatusInfo
//
// Description:
//   Will extract the status information from the byte buffer and invoke the
//   handler.
//
// Arguments:
//   pData: uint8_t* [IN], The raw data.
//   nDataLen: uint16_t [IN], The length of the raw data.
//
// Return values:
//   GOGOCM_UIS__NOERROR: Successful operation.
//   any other value on error.
//
// --------------------------------------------------------------------------
error_t ClientMsgTranslator::TranslateStatusInfo( uint8_t* pData, const uint16_t nDataLen )
{
  gogocStatusInfo statusInfo;
  uint16_t nCursor = 0;
  error_t retCode;


  // ----------------------------------------------
  // Extract the status info from the byte buffer.
  // ----------------------------------------------
  memcpy( (void*)&(statusInfo.eStatus), (void*)pData, sizeof(gogocCliStatus) );
  nCursor += sizeof(gogocCliStatus);

  // Extract message sent along status info.
  memcpy( (void*)&(statusInfo.nStatus), (void*)(pData + nCursor), sizeof(statusInfo.nStatus) );
  nCursor += sizeof(statusInfo.nStatus);


  // -----------------------------------------------------------------------
  // Sanity check. Verify that the bytes of data we extracted match that of
  // what was expected.
  // -----------------------------------------------------------------------
  assert( nCursor == nDataLen );


  // ---------------------------------
  // Invoke derived function handler.
  // ---------------------------------
  retCode = Recv_StatusInfo( &statusInfo );


  // Return completion code.
  return retCode;
}


// --------------------------------------------------------------------------
// Function : TranslateTunnelInfo
//
// Description:
//   Will extract the tunnel information from the byte buffer and invoke the
//   handler.
//
// Arguments:
//   pData: uint8_t* [IN], The raw data.
//   nDataLen: uint16_t [IN], The length of the raw data.
//
// Return values:
//   GOGOCM_UIS__NOERROR: Successful operation.
//   any other value on error.
//
// --------------------------------------------------------------------------
error_t ClientMsgTranslator::TranslateTunnelInfo( uint8_t* pData, const uint16_t nDataLen )
{
  gogocTunnelInfo tunnelInfo;
  uint32_t nCursor = 0;
  error_t retCode;


  // -- D A T A   E X T R A C T I O N --

  // Extract broker name from data buffer.
  tunnelInfo.szBrokerName = pal_strdup( (char*)(pData + nCursor) );
  nCursor += pal_strlen( (char*)(pData + nCursor) ) + 1;

  // Extract tunnel type from data buffer.
  memcpy( (void*)&(tunnelInfo.eTunnelType), pData + nCursor, sizeof(gogocTunnelType) );
  nCursor += sizeof(gogocTunnelType);

  // Extract Local tunnel endpoint IPv4 address from data buffer.
  tunnelInfo.szIPV4AddrLocalEndpoint = pal_strdup( (char*)(pData + nCursor) );
  nCursor += pal_strlen( (char*)(pData + nCursor) ) + 1;

  // Extract Local tunnel endpoint IPv6 address from data buffer.
  tunnelInfo.szIPV6AddrLocalEndpoint = pal_strdup( (char*)(pData + nCursor) );
  nCursor += pal_strlen( (char*)(pData + nCursor) ) + 1;

  // Extract Remote tunnel endpoint IPv4 address from data buffer.
  tunnelInfo.szIPV4AddrRemoteEndpoint = pal_strdup( (char*)(pData + nCursor) );
  nCursor += pal_strlen( (char*)(pData + nCursor) ) + 1;

  // Extract Remote tunnel endpoint IPv6 address from data buffer.
  tunnelInfo.szIPV6AddrRemoteEndpoint = pal_strdup( (char*)(pData + nCursor) );
  nCursor += pal_strlen( (char*)(pData + nCursor) ) + 1;

  // Extract The delegated prefix from data buffer.
  tunnelInfo.szDelegatedPrefix = pal_strdup( (char*)(pData + nCursor) );
  nCursor += pal_strlen( (char*)(pData + nCursor) ) + 1;

  // Extract The delegated user domain from data buffer.
  tunnelInfo.szUserDomain = pal_strdup( (char*)(pData + nCursor) );
  nCursor += pal_strlen( (char*)(pData + nCursor) ) + 1;

  // Extract tunnel uptime from data buffer.
  memcpy( (void*)&(tunnelInfo.tunnelUpTime), pData + nCursor, sizeof(time_t) );
  nCursor += sizeof(time_t);


  // -----------------------------------------------------------------------
  // Sanity check. Verify that the bytes of data we extracted match that of
  // what was expected.
  // -----------------------------------------------------------------------
  assert( nCursor == nDataLen );


  // ---------------------------------
  // Invoke derived function handler.
  // ---------------------------------
  retCode = Recv_TunnelInfo( &tunnelInfo );


  // -----------------------------------------------
  // Clean up allocated memory used for extraction.
  // -----------------------------------------------
  free( tunnelInfo.szBrokerName );
  free( tunnelInfo.szIPV4AddrLocalEndpoint );
  free( tunnelInfo.szIPV6AddrLocalEndpoint );
  free( tunnelInfo.szIPV4AddrRemoteEndpoint );
  free( tunnelInfo.szIPV6AddrRemoteEndpoint );
  free( tunnelInfo.szDelegatedPrefix );
  free( tunnelInfo.szUserDomain );


  // Return completion code.
  return retCode;
}


// --------------------------------------------------------------------------
// Function : TranslateBrokerList
//
// Description:
//   Will extract the broker list from the byte buffer and invoke the
//   handler.
//
// Arguments:
//   pData: uint8_t* [IN], The raw data.
//   nDataLen: uint16_t [IN], The length of the raw data.
//
// Return values:
//   GOGOCM_UIS__NOERROR: Successful operation.
//   any other value on error.
//
// --------------------------------------------------------------------------
gogocBrokerList* BuildBrokerList( uint8_t* pData, const uint32_t nDataLen, uint32_t& nCursor )
{
  if( nCursor >= nDataLen )   // End condition.
    return NULL;

  gogocBrokerList* pList = new gogocBrokerList;
  uint32_t nNameLen = pal_strlen( (const char*)pData ) + 1;
  nCursor += nNameLen;

  pList->szAddress = pal_strdup( (const char*)pData );
  memcpy( (void*)&(pList->nDistance), pData + nCursor, sizeof(int) );
  nCursor += sizeof(int);
  pList->next = BuildBrokerList( pData + nNameLen + sizeof(int), nDataLen, nCursor );

  return pList;
}


// --------------------------------------------------------------------------
error_t ClientMsgTranslator::TranslateBrokerList( uint8_t* pData, const uint16_t nDataLen )
{
  gogocBrokerList* pList;
  uint32_t nCursor = 0;
  error_t retCode;


  // ------------------------------------------
  // Extract broker list from the byte buffer.
  // ------------------------------------------
  pList = BuildBrokerList( pData, nDataLen, nCursor );


  // -----------------------------------------------------------------------
  // Sanity check. Verify that the bytes of data we extracted match that of
  // what was expected.
  // -----------------------------------------------------------------------
  assert( nCursor == nDataLen );


  // ---------------------------------
  // Invoke derived function handler.
  // ---------------------------------
  retCode = Recv_BrokerList( pList );


  // -----------------------------------------------
  // Clean up allocated memory used for extraction.
  // -----------------------------------------------
  gogocBrokerList* pListLast = NULL;
  for(; pList!=NULL; pList = pList->next )
  {
    if( pListLast != NULL ) delete pListLast;
    free( pList->szAddress );
    pListLast = pList;
  }
  if( pListLast != NULL ) delete pListLast;


  // Return completion status.
  return retCode;
}


// --------------------------------------------------------------------------
// Function : TranslateHACCESSStatusInfo
//
// Description:
//   Will extract the HACCESS Status Info from the byte buffer and invoke the
//   handler.
//
// Arguments:
//   pData: uint8_t* [IN], The raw data.
//   nDataLen: uint16_t [IN], The length of the raw data.
//
// Return values:
//   GOGOCM_UIS__NOERROR: Successful operation.
//   any other value on error.
//
// --------------------------------------------------------------------------
PMAPPING_STATUS BuildMappingStatuses( uint8_t* pData, const uint32_t nDataLen, uint32_t& nCursor )
{
  if( nCursor >= nDataLen )   // End condition.
    return NULL;

  PMAPPING_STATUS pMappingStatus = new MAPPING_STATUS;

  // Extract Device Name
  pMappingStatus->device_name = pal_strdup( (const char*)pData );
  uint32_t nNameLen = pal_strlen( (const char*)pData ) + 1;
  nCursor += nNameLen;

  // Extract Device mapping status
  memcpy( (void*)&(pMappingStatus->mapping_status), pData + nNameLen, sizeof(HACCESSDevMapStts) );
  nCursor += sizeof(int);

  // Extract the next.
  pMappingStatus->next = BuildMappingStatuses( pData + nNameLen + sizeof(int), nDataLen, nCursor );

  return pMappingStatus;
}

void FreeMappingStatus( PMAPPING_STATUS mapStat )
{
  if( mapStat != NULL )
  {
    FreeMappingStatus( mapStat->next );
    assert( mapStat->device_name != NULL );
    delete [] mapStat->device_name;
    delete mapStat;
  }
}

error_t ClientMsgTranslator::TranslateHACCESSStatusInfo( uint8_t* pData, const uint16_t nDataLen )
{
  HACCESSStatusInfo statusInfo;
  uint32_t nCursor = 0;
  error_t retCode;


  // -- D A T A   E X T R A C T I O N --

  // Extract the proxy status from data buffer.
  memcpy( (void*)&(statusInfo.haccess_proxy_status), pData + nCursor, sizeof(HACCESSFeatStts) );
  nCursor += sizeof(HACCESSFeatStts);

  // Extract the web status from data buffer.
  memcpy( (void*)&(statusInfo.haccess_web_status), pData + nCursor, sizeof(HACCESSFeatStts) );
  nCursor += sizeof(HACCESSFeatStts);

  // Extract the Device Mapping Module status from data buffer.
  memcpy( (void*)&(statusInfo.haccess_devmapmod_status), pData + nCursor, sizeof(HACCESSFeatStts) );
  nCursor += sizeof(HACCESSFeatStts);

  // Extract the device mapping statuses. Unserialize the linked list.
  statusInfo.haccess_devmap_statuses = BuildMappingStatuses( pData + nCursor, nDataLen, nCursor );


  // -----------------------------------------------------------------------
  // Sanity check. Verify that the bytes of data we extracted match that of
  // what was expected.
  // -----------------------------------------------------------------------
  assert( nCursor == nDataLen );


  // ---------------------------------
  // Invoke derived function handler.
  // ---------------------------------
  retCode = Recv_HACCESSStatusInfo( &statusInfo );


  // -----------------------------------------------
  // Clean up allocated memory used for extraction.
  // -----------------------------------------------
  FreeMappingStatus( statusInfo.haccess_devmap_statuses );


  // Return completion code.
  return retCode;
}


} // namespace
