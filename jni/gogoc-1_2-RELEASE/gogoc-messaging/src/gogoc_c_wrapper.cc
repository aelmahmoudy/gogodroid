// **************************************************************************
// $Id: gogoc_c_wrapper.cc,v 1.1 2009/11/20 16:34:55 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Implementation of the C function wrappers.
//
// Author: Charles Nepveu
//
// Creation Date: December 2006
// __________________________________________________________________________
// **************************************************************************
#include <gogocmessaging/gogoc_c_wrapper.h>
#include <gogocmessaging/clientmsgdataretriever.h>
#include <gogocmessaging/clientmessengerimpl.h>
using namespace gogocmessaging;


// The unique instance of the gogoCLIENT Messenger implementation object.
static ClientMessengerImpl* pMessenger = NULL;



// --------------------------------------------------------------------------
// Function : initialize_messaging
//
// Description:
//   Will instantiate the gogoCLIENT Messenger Impl object, thus providing
//   Messenger capabilities to this process.
//   This implementation ensures that only one messenger object exists at a 
//   time.
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR: Successful completion.
//   GOGOCM_UIS_CWRAPALRDYINIT: Initialization procedure previously called.
//
// --------------------------------------------------------------------------
extern "C" error_t initialize_messaging( void )
{
  if( pMessenger != NULL )
    return GOGOCM_UIS_CWRAPALRDYINIT;

  // Instantiate a new Client Messenger.
  pMessenger = new ClientMessengerImpl();

  // It is preferable to wait until the worker threads of the 
  // client messenger are ready.
  // -> Wait UP TO 750 miliseconds...
  pMessenger->WaitReady( 750 );     // Should be done within 1 or 2 ms.


  return GOGOCM_UIS__NOERROR;
}


// --------------------------------------------------------------------------
// Function : uninitialize_messaging
//
// Description:
//   Will destroy the gogoCLIENT Messenger Impl object, thus stopping 
//   Messenger capabilities to this process.
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR: Successful completion.
//   GOGOCM_UIS_CWRAPNOTINIT: Messenger object had not been initialized.
//
// --------------------------------------------------------------------------
extern "C" error_t uninitialize_messaging( void )
{
  if( pMessenger == NULL )
    return GOGOCM_UIS_CWRAPNOTINIT;

  delete pMessenger;
  pMessenger = NULL;

  return GOGOCM_UIS__NOERROR;
}


// --------------------------------------------------------------------------
// Function : send_status_info
//
// Description:
//   Sends status info to the GUI (or whichever client that's connected).
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR: Successful completion.
//   GOGOCM_UIS_CWRAPNOTINIT: Messenger not initialized.
//
// --------------------------------------------------------------------------
extern "C" error_t send_status_info( void )
{
  gogocStatusInfo* pStatusInfo = NULL;
  error_t retCode = GOGOCM_UIS__NOERROR;


  // Verify if messenger object has been initialized.
  if( pMessenger == NULL )
    return GOGOCM_UIS_CWRAPNOTINIT;

  // Callback to the gogoCLIENT process, to gather required information.
  retCode = RetrieveStatusInfo( &pStatusInfo );
  if( retCode == GOGOCM_UIS__NOERROR )
  {
    // Send the tunnel info to the other side.
    pMessenger->Send_StatusInfo( pStatusInfo );

    // Frees the memory used by the StatusInfo object.
    FreeStatusInfo( &pStatusInfo );
  }

  return retCode;
}


// --------------------------------------------------------------------------
// Function : send_tunnel_info
//
// Description:
//   Sends tunnel info to the GUI (or whichever client that's connected).
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR: Successful completion.
//   GOGOCM_UIS_CWRAPNOTINIT: Messenger not initialized.
//
// --------------------------------------------------------------------------
extern "C" error_t send_tunnel_info( void )
{
  gogocTunnelInfo* pTunnelInfo = NULL;
  error_t retCode = GOGOCM_UIS__NOERROR;


  // Verify if messenger object has been initialized.
  if( pMessenger == NULL )
    return GOGOCM_UIS_CWRAPNOTINIT;

  // Callback to the gogoCLIENT process, to gather required information.
  retCode = RetrieveTunnelInfo( &pTunnelInfo );
  if( retCode == GOGOCM_UIS__NOERROR )
  {
    // Send the tunnel info to the other side.
    pMessenger->Send_TunnelInfo( pTunnelInfo );

    // Frees the memory used by the TunnelInfo object.
    FreeTunnelInfo( &pTunnelInfo );
  }

  return retCode;
}


// --------------------------------------------------------------------------
// Function : send_broker_list
//
// Description:
//   Sends broker list to the GUI (or whichever client that's connected).
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR: Successful completion.
//   GOGOCM_UIS_CWRAPNOTINIT: Messenger not initialized.
//
// --------------------------------------------------------------------------
extern "C" error_t send_broker_list( void )
{
  gogocBrokerList* pBrokerList = NULL;
  error_t retCode = GOGOCM_UIS__NOERROR;


  // Verify if messenger object has been initialized.
  if( pMessenger == NULL )
    return GOGOCM_UIS_CWRAPNOTINIT;

  // Callback to the gogoCLIENT process, to gather required information.
  retCode = RetrieveBrokerList( &pBrokerList );
  if( retCode == GOGOCM_UIS__NOERROR )
  {
    // Send the broker list to the other side.
    pMessenger->Send_BrokerList( pBrokerList );

    // Frees the memory used by the BrokerList object.
    FreeBrokerList( &pBrokerList );
  }

  return retCode;
}


// --------------------------------------------------------------------------
// Function : send_haccess_status_info
//
// Description:
//   Sends HACCESS status info to the GUI (or whichever client that's connected).
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR: Successful completion.
//   GOGOCM_UIS_CWRAPNOTINIT: Messenger not initialized.
//
// --------------------------------------------------------------------------
extern "C" error_t send_haccess_status_info( void )
{
  HACCESSStatusInfo* pHACCESSStatusInfo = NULL;
  error_t retCode = GOGOCM_UIS__NOERROR;


  // Verify if messenger object has been initialized.
  if( pMessenger == NULL )
    return GOGOCM_UIS_CWRAPNOTINIT;

  // Callback to the gogoCLIENT process, to gather required information.
  retCode = RetrieveHACCESSStatusInfo( &pHACCESSStatusInfo );
  if( retCode == GOGOCM_UIS__NOERROR )
  {
    // Send the HACCESS status info to the other side.
    pMessenger->Send_HACCESSStatusInfo( pHACCESSStatusInfo );

    // Frees the memory used by the HACCESSStatusInfo object.
    FreeHACCESSStatusInfo( &pHACCESSStatusInfo );
  }

  return retCode;
}



