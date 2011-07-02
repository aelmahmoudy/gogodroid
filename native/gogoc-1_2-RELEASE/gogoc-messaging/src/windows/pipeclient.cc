// **************************************************************************
// $Id: pipeclient.cc,v 1.1 2009/11/20 16:34:58 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Windows implementation of the PipeClient class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gogocmessaging/pipeclient.h>
#include <windows.h>
#include <assert.h>


#define PIPE_BUFSIZ           4096
#define PIPE_DFLT_TIMEOUT     200     // miliseconds


namespace gogocmessaging
{
// --------------------------------------------------------------------------
// Function : PipeClient constructor
//
// Description:
//   Will initialize a new PipeClient object.
//
// Arguments:
//   aPipeName: string [IN], The name of the pipe this client will 
//              connect to.
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
PipeClient::PipeClient( const string& aPipeName ) :
  IPCClient(),
  PipeIO(),
  m_PipeName(aPipeName)
{
  assert( !aPipeName.empty() );
}


// --------------------------------------------------------------------------
// Function : PipeClient destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
PipeClient::~PipeClient( void )
{
  // Is client pipe still online ?
  assert( m_Handle == INVALID_IPC_HANDLE );
}


// --------------------------------------------------------------------------
// Function : Connect
//
// Description:
//   Will establish a client pipe with the server.
//   NOTE: THIS FUNCTION IS (big time) BLOCKING!!!
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR: Operation successful.
//   Any other value is an error. Use GetLastError() for more information.
//
// --------------------------------------------------------------------------
error_t PipeClient::Connect( void )
{
  // Check if client pipe handle is valid.
  if( m_Handle != INVALID_IPC_HANDLE )
    return GOGOCM_UIS_CLIENTALRDYCONN;

  // Loop until we're connected.
  while( m_Handle == INVALID_IPC_HANDLE )
  {
    // Check if server pipe endpoint is available.
    if( WaitNamedPipe( m_PipeName.c_str(), NMPWAIT_WAIT_FOREVER ) == TRUE )
    {
      // Connect to server endpoint.
      m_Handle = (IPC_HANDLE)CreateFile( m_PipeName.c_str(),
                                 GENERIC_READ | GENERIC_WRITE,
                                 0,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL );

      // Validate returned value.
      if( m_Handle == INVALID_IPC_HANDLE )
      {
        return GOGOCM_UIS_FAILCREATECLIENTPIPE;
      }

      // Specify client transfer mode: MESSAGES
      DWORD dwMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
      if( SetNamedPipeHandleState( m_Handle, &dwMode, NULL, NULL ) == 0 )
      {
        CloseHandle( m_Handle );
        m_Handle = INVALID_IPC_HANDLE;
        return GOGOCM_UIS_FAILCREATECLIENTPIPE;
      }
    }
    else
    {
      // If no instances of the specified named pipe exist, the 
      // WaitNamedPipe function returns immediately, regardless of 
      // the time-out value.
      Sleep(20);      // Wait a bit before retrying.

      // Failure to sleep will cause 100% CPU usage.
    }
  }

  // Connection to server successful.
  return GOGOCM_UIS__NOERROR;
}


// --------------------------------------------------------------------------
// Function : Disconnect
//
// Description:
//   Will terminate connection to the pipe server.
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR: Operation successful.
//   Any other value is an error. Use GetLastError() for more information.
//
// --------------------------------------------------------------------------
error_t PipeClient::Disconnect( void )
{
  // Check if client pipe handle is valid.
  if( m_Handle == INVALID_IPC_HANDLE )
    return GOGOCM_UIS_PIPECLIDISCFAIL;

  // Close connection to server.
  if( CloseHandle( m_Handle ) == 0 )
  {
    return GOGOCM_UIS_PIPECLIDISCFAIL;
  }
  m_Handle = INVALID_IPC_HANDLE;

  // Disconnection successful.
  return GOGOCM_UIS__NOERROR;
}

}
