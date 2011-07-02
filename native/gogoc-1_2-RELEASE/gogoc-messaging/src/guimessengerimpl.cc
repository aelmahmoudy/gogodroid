// **************************************************************************
// $Id: guimessengerimpl.cc,v 1.1 2009/11/20 16:34:56 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Implementation of the GUIMessengerImpl class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gogocmessaging/guimessengerimpl.h>
#include <gogocmessaging/gogocmsgdata.h>


namespace gogocmessaging
{
// --------------------------------------------------------------------------
// Function : GUIMessengerImpl constructor
//
// Description:
//   Will initialize a new GUIMessengerImpl object.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
GUIMessengerImpl::GUIMessengerImpl( void ) :
  ClientMsgSender(), 
  ClientMsgTranslator(),
  m_RecvStatusInfo(NULL),
  m_RecvTunnelInfo(NULL),
  m_RecvBrokerList(NULL),
  m_RecvHACCESSStatusInfo(NULL),
  m_CommManager( CLIENT_MANAGER, this )
{
  // Message processing is enabled by default in ClientMsgTranslator.
  m_CommManager.Run();
}


// --------------------------------------------------------------------------
// Function : GUIMessengerImpl destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
GUIMessengerImpl::~GUIMessengerImpl( void )
{
}


// --------------------------------------------------------------------------
// Function : DisableProcessing
//
// Description:
//   Will disable message processing. Incoming messages will be dropped.
//   Useful when you want callbacks to stop being invoked.
//   NOTE: This does not disable message sending.
//
// Arguments: (none)
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void GUIMessengerImpl::DisableProcessing( void )
{
  MessageProcessor::m_eProcessorState = STATE_DISABLED;
}


// --------------------------------------------------------------------------
// Function : EnableProcessing
//
// Description:
//   Will enable(resume) message processing.
//
// Arguments: (none)
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void GUIMessengerImpl::EnableProcessing( void )
{
  MessageProcessor::m_eProcessorState = STATE_ENABLED;
}


// --------------------------------------------------------------------------
// Function : Recv_StatusInfo
//
// Description:
//   Invoked upon reception of a StatusInfo message from the Communications
//   Manager. The information contains the GOGOClient status.
//
// Arguments:
//   aStatusInfo: gogocStatusInfo* [IN], The client status.
//
// Return values:
//   GOGOCM_UIS__NOERROR: Indicates success replying to request.
//
// --------------------------------------------------------------------------
error_t GUIMessengerImpl::Recv_StatusInfo( const gogocStatusInfo* aStatusInfo )
{
  // Callback the provided function.
  if( m_RecvStatusInfo != NULL )
    (*m_RecvStatusInfo)( aStatusInfo );

  return GOGOCM_UIS__NOERROR;
}


// --------------------------------------------------------------------------
// Function : Recv_TunnelInfo
//
// Description:
//   Invoked upon reception of a TunnelInfo message from the Communications
//   Manager. The information contains the GOGOClient tunnel info.
//
// Arguments:
//   aTunnelInfo: gogocTunnelInfo* [IN], The tunnel information.
//
// Return values:
//   GOGOCM_UIS__NOERROR: Indicates success replying to request.
//
// --------------------------------------------------------------------------
error_t GUIMessengerImpl::Recv_TunnelInfo( const gogocTunnelInfo* aTunnelInfo )
{
  // Callback the provided function.
  if( m_RecvTunnelInfo != NULL )
    (*m_RecvTunnelInfo)(aTunnelInfo);

  return GOGOCM_UIS__NOERROR;
}


// --------------------------------------------------------------------------
// Function : Recv_BrokerList
//
// Description:
//   Invoked upon reception of a BrokerList message from the Communications
//   Manager. The information contains the GOGOClient list of brokers.
//
// Arguments:
//   aBrokerList: gogocBrokerList* [IN], The list of brokers.
//
// Return values:
//   GOGOCM_UIS__NOERROR: Indicates success replying to request.
//
// --------------------------------------------------------------------------
error_t GUIMessengerImpl::Recv_BrokerList( const gogocBrokerList* aBrokerList )
{
  // Callback the provided function.
  if( m_RecvBrokerList != NULL )
    (*m_RecvBrokerList)(aBrokerList);

  return GOGOCM_UIS__NOERROR;
}


// --------------------------------------------------------------------------
// Function : Recv_HACCESSStatusInfo
//
// Description:
//   Invoked upon reception of a BrokerList message from the Communications
//   Manager. The information contains the statuses of the HACCESS features.
//
// Arguments:
//   aBrokerList: aHACCESSStatusInfo* [IN], The HACCESS status information.
//
// Return values:
//   GOGOCM_UIS__NOERROR: Indicates success replying to request.
//
// --------------------------------------------------------------------------
error_t GUIMessengerImpl::Recv_HACCESSStatusInfo( const HACCESSStatusInfo* aHACCESSStatusInfo )
{
  // Callback the provided function.
  if( m_RecvHACCESSStatusInfo != NULL )
    (*m_RecvHACCESSStatusInfo)(aHACCESSStatusInfo);

  return GOGOCM_UIS__NOERROR;
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
void GUIMessengerImpl::PostMessage( Message* pMsg )
{
  MessageSender* pSender = (MessageSender*) &m_CommManager;
  pSender->PostMessage( pMsg );
}

} // namespace
