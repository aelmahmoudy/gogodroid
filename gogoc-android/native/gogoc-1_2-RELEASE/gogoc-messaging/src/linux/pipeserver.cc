// **************************************************************************
// $Id: pipeserver.cc,v 1.1 2009/11/20 16:34:57 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Unix implementation of the PipeServer class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gogocmessaging/pipeserver.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#error THIS MODULE IS NOT FINISHED

#define PIPE_MASK              0666
#define SLEEP(X)               usleep( X * 1000 )


typedef void (*sig_handler)(int);
extern "C" void sigpipe_handler( int whatever )
{

}


namespace gogocmessaging
{
// --------------------------------------------------------------------------
// Function : PipeServer constructor
//
// Description:
//   Will initialize a new PipeServer object.
//
// Arguments:
//   aPipeName: string [IN], The name of the pipe this server will create.
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
PipeServer::PipeServer( const string& aPipeName ) :
  IPCServer(),
  PipeIO(),
  m_PipeName(aPipeName),
  m_bClientConnected(false)
{
}


// --------------------------------------------------------------------------
// Function : PipeServer destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
PipeServer::~PipeServer( void )
{
  // Is client still connected ?
  assert( m_bClientConnected == false );
}


// --------------------------------------------------------------------------
// Function : CreateConnectionPoint
//
// Description:
//   Will create a NamedPipe server endpoint.
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR: Operation successful.
//   Any other value is an error. Use GetLastError() for more information.
//
// --------------------------------------------------------------------------
error_t PipeServer::CreateConnectionPoint( void )
{
  assert( !m_PipeName.empty() );


  // Clear umask so it doesn't interfere with pipe security mask.
  umask(0);


  // ---------------------------------
  // Create the server pipe endpoint.
  // ---------------------------------
  int retCode = mknod( m_PipeName.c_str(), S_IFIFO|PIPE_MASK, 0);


  // Verify returned value.
  if( retCode != 0 )
  { 
    return GOGOCM_UIS_FAILCREATESERVERPIPE;
  }

  // Successful operation.
  return GOGOCM_UIS__NOERROR;
}


// --------------------------------------------------------------------------
// Function : AcceptConnection
//
// Description:
//   Will accept a pipe client connection.
//   NOTE: THIS FUNCTION IS (big time) BLOCKING!!!
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR: Operation successful.
//   Any other value is an error. Use GetLastError() for more information.
//
// --------------------------------------------------------------------------
error_t PipeServer::AcceptConnection( void )
{
  FILE* hPipe;


  if( m_bClientConnected )
    return GOGOCM_UIS_CLIENTALRDYCONN;


  // Open the pipe in read - write mode, and block until
  // a client connects.
  //
  hPipe = fopen( m_PipeName.c_str(), "rwb" );
  if( hPipe == NULL )
  {
    // Error opening pipe.
    return GOGOCM_UIS_FAILOPENPIPE;
  }


  // Register new handler for SIG_PIPE signal.
  sig_handler oldpipehandler = signal( SIGPIPE, sigpipe_handler );
  if( oldpipehandler == SIG_ERR )
  {

  } 


  while( !m_bClientConnected )
  {

     SLEEP( 30 );

  }

  // Client connection successful
  return GOGOCM_UIS__NOERROR;
}


// --------------------------------------------------------------------------
// Function : CloseConnection
//
// Description:
//   Will close the server-side pipe instance.
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR: Operation successful.
//   Any other value is an error. Use GetLastError() for more information.
//
// --------------------------------------------------------------------------
error_t PipeServer::CloseConnection( void )
{
  // Mark client as not connected.
  m_bClientConnected = false;

  // Server pipe disconnection successful.
  return GOGOCM_UIS__NOERROR;
}

}
