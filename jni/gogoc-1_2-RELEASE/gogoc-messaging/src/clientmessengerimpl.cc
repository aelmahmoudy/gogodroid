// **************************************************************************
// $Id: clientmessengerimpl.cc,v 1.1 2009/11/20 16:34:55 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Implementation of the ClientMessengerImpl class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gogocmessaging/clientmessengerimpl.h>
#include <gogocmessaging/clientmsgdataretriever.h>
#include <gogocmessaging/clientmsgnotifier.h>


namespace gogocmessaging
{
// --------------------------------------------------------------------------
// Function : ClientMessengerImpl constructor
//
// Description:
//   Will initialize a new ClientMessengerImpl object.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
ClientMessengerImpl::ClientMessengerImpl( void ) :
  ServerMsgSender(), 
  ServerMsgTranslator(),
  m_CommManager( SERVER_MANAGER, this )
{
  m_CommManager.Run();
}


// --------------------------------------------------------------------------
// Function : ClientMessengerImpl destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
ClientMessengerImpl::~ClientMessengerImpl( void )
{
}


// --------------------------------------------------------------------------
// Function : Recv_StatusInfoRequest
//
// Description:
//   Will wait on the communication manager to get ready. If ready state is 
//   not signalled within XXX miliseconds, this function exits.
//
// Arguments:
//   ulWaitms: unsigned long [IN], the time to wait for ready state.
//
// Return values:
//   true: communications manager signalled ready state within XX miliseconds
///  false: ready state was not signalled within the XX miliseconds.
//
// --------------------------------------------------------------------------
bool ClientMessengerImpl::WaitReady( unsigned long ulWaitms )
{
  return m_CommManager.WaitReady( ulWaitms );
}


// --------------------------------------------------------------------------
// Function : Recv_StatusInfoRequest
//
// Description:
//   Will gather information on the gogoCLIENT status and send it to the
//   requester.
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR: Indicates success replying to request.
//
// --------------------------------------------------------------------------
error_t ClientMessengerImpl::Recv_StatusInfoRequest( void )
{
  gogocStatusInfo* pStatusInfo = NULL;
  error_t retCode;


  // Retrieve the status information. (ALLOCATION MADE HERE)
  retCode = RetrieveStatusInfo( &pStatusInfo );
  if( retCode == GOGOCM_UIS__NOERROR )
  {
    // Send the information to the requester.
    Send_StatusInfo( pStatusInfo );

    // Free memory allocated.
    FreeStatusInfo( &pStatusInfo );
  }

  return retCode;
}


// --------------------------------------------------------------------------
// Function : Recv_TunnelInfoRequest
//
// Description:
//   Will gather information on the tunnel and send it to the requester.
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR: Indicates success replying to request.
//
// --------------------------------------------------------------------------
error_t ClientMessengerImpl::Recv_TunnelInfoRequest( void )
{
  gogocTunnelInfo* pTunnelInfo = NULL;
  error_t retCode;


  // Retrieve the status information. (ALLOCATION MADE HERE)
  retCode = RetrieveTunnelInfo( &pTunnelInfo );
  if( retCode == GOGOCM_UIS__NOERROR )
  {
    // Send the information to the requester.
    Send_TunnelInfo( pTunnelInfo );

    // Free memory allocated.
    FreeTunnelInfo( &pTunnelInfo );
  }

  return retCode;
}


// --------------------------------------------------------------------------
// Function : Recv_BrokerListRequest
//
// Description:
//   Will gather the information on the list of brokers, and put it in a list
//   and sent it to the requester.
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR: Indicates success receiving request.
//
// --------------------------------------------------------------------------
error_t ClientMessengerImpl::Recv_BrokerListRequest( void )
{
  gogocBrokerList* pBrokerList = NULL;
  error_t retCode;


  // Retrieve the broker list. (ALLOCATION MADE HERE)
  retCode = RetrieveBrokerList( &pBrokerList );
  if( retCode == GOGOCM_UIS__NOERROR )
  {
    // Send the information to the requester.
    Send_BrokerList( pBrokerList );

    // Free memory allocated.
    FreeBrokerList( &pBrokerList );
  }

  return retCode;
}


// --------------------------------------------------------------------------
// Function : Recv_HACCESSConfigInfo
//
// Description:
//   Will call the C function that is implemented in the GOGOC.
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR: Indicates success receiving message.
//
// --------------------------------------------------------------------------
error_t ClientMessengerImpl::Recv_HACCESSConfigInfo( const HACCESSConfigInfo* aHACCESSConfigInfo )
{
  // C++ -> C bridge

  // This C function is implemented in the gogoCLIENT.
  return NotifyhaccessConfigInfo( aHACCESSConfigInfo );
}


// --------------------------------------------------------------------------
// Function : Recv_HACCESSStatusInfoRequest
//
// Description:
//   Will gather information on the HACCESS Status and send it to the requester.
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR: Indicates success replying to request.
//
// --------------------------------------------------------------------------
error_t ClientMessengerImpl::Recv_HACCESSStatusInfoRequest( void )
{
  HACCESSStatusInfo* pHACCESSStatusInfo = NULL;
  error_t retCode;


  // Retrieve the status information. (ALLOCATION MADE HERE)
  retCode = RetrieveHACCESSStatusInfo( &pHACCESSStatusInfo );
  if( retCode == GOGOCM_UIS__NOERROR )
  {
    // Send the information to the requester.
    Send_HACCESSStatusInfo( pHACCESSStatusInfo );

    // Free memory allocated.
    FreeHACCESSStatusInfo( &pHACCESSStatusInfo );
  }

  return retCode;
}


// --------------------------------------------------------------------------
// Function : PostMessage
//
// Description:
//   Will post a message to the send queue for sending.
//
// Arguments:
//   pMsg: Message* [IN], The message to post.
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void ClientMessengerImpl::PostMessage( Message* pMsg )
{
  MessageSender* pSender = (MessageSender*) &m_CommManager;
  pSender->PostMessage( pMsg );
}

} // namespace
