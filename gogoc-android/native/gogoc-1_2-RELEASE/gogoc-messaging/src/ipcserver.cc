// **************************************************************************
// $Id: ipcserver.cc,v 1.1 2009/11/20 16:34:56 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//  Implementation of the IPCServer class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <pal.h>
#include <gogocmessaging/ipcserver.h>
#include <assert.h>


namespace gogocmessaging
{
// --------------------------------------------------------------------------
// Function : IPCServer constructor
//
// Description:
//   Will initialize a new IPCServer object.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
IPCServer::IPCServer( void ) :
  IPCServent()
{
  // Semaphore is locked initially.
  m_pSemReadyState = new Semaphore( 1, 0 );
  assert( m_pSemReadyState != NULL );
}


// --------------------------------------------------------------------------
// Function : IPCServer destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
IPCServer::~IPCServer( void )
{
  if( m_pSemReadyState != NULL )
  {
    delete m_pSemReadyState;
    m_pSemReadyState = NULL;
  }
}


// --------------------------------------------------------------------------
// Function : Initialize
//
// Description:
//   Will initialize the IPC Server by setting up a connection and waiting
//   for an IPC client to connect.
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR on success
//   See CreateConnectionPoint() and AcceptConnection() specializations.
//
// --------------------------------------------------------------------------
error_t IPCServer::Initialize( void )
{
  error_t nRetCode;

  if( (nRetCode = CreateConnectionPoint()) == GOGOCM_UIS__NOERROR )
  {
    // SIGNAL THE READY STATE
    m_pSemReadyState->ReleaseLock();

    return AcceptConnection();
  }

  return nRetCode;
}


// --------------------------------------------------------------------------
// Function : UnInitialize
//
// Description:
//   Will terminate the connection with the IPC client and shut down.
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR on success
//   See CloseConnection() specialization.
//
// --------------------------------------------------------------------------
error_t IPCServer::UnInitialize( void )
{
  return CloseConnection();
}


// --------------------------------------------------------------------------
// Function : WaitReady
//
// Description:
//   Will wait until the m_pSemReadyState semaphore is released.
//   The semaphore is released when the server has created the connection
//   point and is ready to accept incomming connection(s).
//
// Arguments:
//   ulWaitms: unsigned long [IN], The timeout for the wait operation.
//             If 0, the timeout is INFINITE.
//
// Return values:
//   Returns true if the semaphore is released before the specified timeout.
//
// --------------------------------------------------------------------------
bool IPCServer::WaitReady( unsigned long ulWaitms )
{
  bool bRetVal;

  if( bRetVal = (m_pSemReadyState->WaitAndLock( ulWaitms ) == 0) )
    m_pSemReadyState->ReleaseLock();

  return bRetVal;
}

}
