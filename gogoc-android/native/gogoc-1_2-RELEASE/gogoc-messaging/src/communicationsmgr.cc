// **************************************************************************
// $Id: communicationsmgr.cc,v 1.1 2009/11/20 16:34:55 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
//
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Implementation of the CommunicationsManager class,
//   and the Sender and Receiver worker threads.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gogocmessaging/communicationsmgr.h>
#include <gogocmessaging/pipeclient.h>
#include <gogocmessaging/pipeserver.h>
#include <gogocmessaging/debugdefs.h>


// --------------------------------------------
// -- GATEWAY6 CLIENT IPC CHANNEL: PIPE NAME --
// --------------------------------------------
#define COMMMGR_PIPE_NAME   "\\\\.\\pipe\\gogocmessaging-ipc"

#define INITERRORS_THRESHOLD  3   // Number of failed initialization berore giving up.


namespace gogocmessaging
{
// --------------------------------------------------------------------------
// Function : CommunicationsManager constructor
//
// Description:
//   Will initialize a new CommunicationsManager object.
//
// Arguments:
//   aEMode: enum [IN], The communications manager operation mode.
//   aProcessor: MessageProcessor* [IN], A message receiver-derived class.
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
CommunicationsManager::CommunicationsManager( tManagerMode aEMode, MessageProcessor* aProcessor ) :
  MessageSender(),
  ThreadWrapper(),
  m_IPCServent( NULL ),
  m_MsgProcessor( aProcessor ),
  m_ReceiverThread( NULL ),
  m_SenderThread( NULL ),
  m_eManagerMode( aEMode ),
  m_nMsgSent(0),
  m_nMsgProcessed(0)
{
  assert( aProcessor != NULL );
  SetState( STATE_DISCONNECTED );
}


// --------------------------------------------------------------------------
// Function : CommunicationsManager destructor
//
// Description:
//   Will stop working threads and clean-up space allocated during object
//   lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
CommunicationsManager::~CommunicationsManager( void )
{
  // Stop the worker thread.
  Stop();

  // Clean up instance and stop other threads.
  _CleanupInstance();
}


// --------------------------------------------------------------------------
// Function : Initialize
//
// Description:
//   Will create the IPC Servent object depending on manager operation mode
//   and initialize the Servent object for future IO operations.
//   The receiver and sender worker threads will be initialized and started.
//
// Arguments: (none)
//
// Return values:
//   GOGOCM_UIS__NOERROR: On successful initialization.
//   any other error code on error.
//
// --------------------------------------------------------------------------
error_t CommunicationsManager::Initialize( void )
{
  error_t retCode;
  assert( m_eManagerStatus == STATE_PENDINGCONNECTION );
  assert( m_IPCServent == NULL );
  assert( m_SenderThread == NULL );
  assert( m_ReceiverThread == NULL );


  // -------------------------------------------------------------
  // Create the IPC servent, depending on manager operation mode.
  // -------------------------------------------------------------
  switch( m_eManagerMode )
  {
  case CLIENT_MANAGER:
    m_IPCServent = new PipeClient( COMMMGR_PIPE_NAME );
    break;

  case SERVER_MANAGER:
    m_IPCServent = new PipeServer( COMMMGR_PIPE_NAME );
    break;
  }
  assert( m_IPCServent != NULL );


  // ---------------------------------------------------------------
  // Initialize Servent oject:      [ BLOCKING ]
  //   In server mode, il will wait for a client to connect.
  //   In client mode, it will wait for the server to be available.
  // ---------------------------------------------------------------
  if( (retCode = m_Servent.Initialize( m_IPCServent )) != 0 )
  {
    DBG_PRINT( "Failed to Initialize server Servent object. Error code: "
         << setbase(16) << retCode << setbase(10) );
    return retCode;
  }


  // ---------------------------------
  // Start the message sender thread.
  //   See SenderThread::Work()
  // ---------------------------------
  m_SenderThread = new SenderThread( this );
  m_SenderThread->Run();


  // ---------------------------------
  // Start the message receiver thread.
  //   See ReceiverThread::Work()
  // ---------------------------------
  m_ReceiverThread = new ReceiverThread( this );
  m_ReceiverThread->Run();


  return GOGOCM_UIS__NOERROR;
}


// --------------------------------------------------------------------------
// Function : _CleanupInstance        [ PRIVATE ]
//
// Description:
//   Will stop working threads and clean-up space allocated during object
//   lifetime.
//
// Arguments: (none)
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void CommunicationsManager::_CleanupInstance( void )
{
  // ----------------------------------------
  // Stop Sender and Receiver threads first.
  // ----------------------------------------
  if( m_SenderThread != NULL )
  {
    m_SenderThread->Stop();
    delete m_SenderThread;
    m_SenderThread = NULL;
  }

  if( m_ReceiverThread != NULL )
  {
    m_ReceiverThread->Stop();
    delete m_ReceiverThread;
    m_ReceiverThread = NULL;
  }


  // Deallocate IPC Servent object.
  if( m_IPCServent != NULL )
  {
    m_IPCServent->UnInitialize();
    delete m_IPCServent;
    m_IPCServent = NULL;
  }

  // Empty send queue and reset semaphore on SendQueue.
  MessageSender::Reset();
}


// --------------------------------------------------------------------------
// Function : CommunicationsManager::Work   [ THREAD ]
//
// Description:
//   This thread will monitor the state of the IPC layer components.
//   If a layer component should fail (e.g.: broken IPC, disconnection, etc),
//   a reinitialization will occur to re-establish connection and get back
//   online.
//   This function will exit if repetitive errors occurs.
//
// Arguments: (none)
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void CommunicationsManager::Work( void )
{
  unsigned short nErrorCount=0;
  error_t retCode;


  DBG_PRINT( "Communications manager service thread started." );


  // ------------------------------------------------
  // Communications manager service management loop.
  // ------------------------------------------------
  while( !ShouldStop()  &&  GetState() != STATE_FATALERROR )
  {
    // -------------------------------------------------------
    // Check if current manager status is STATE_DISCONNECTED.
    // -------------------------------------------------------
    if( GetState() == STATE_DISCONNECTED )
    {
      // ------------------------------------------------------------
      // Clean up IPC layer objects instances and ready for restart.
      // ------------------------------------------------------------
      _CleanupInstance();


      // ------------------------------
      // Run initialization procedure.
      // ------------------------------
      SetState( STATE_PENDINGCONNECTION );
      if( (retCode = Initialize()) == GOGOCM_UIS__NOERROR )
      {
        // Reset error count.
        nErrorCount = 0;

        // Successful initialization.
        SetState( STATE_CONNECTED );
      }
      else
      {
        // -----------------------
        // Failed initialization.
        // -----------------------
        DBG_PRINT( "Failed Initialization." << endl << "Error code: " <<
                setbase(16) << retCode << setbase(10) );
#ifdef WIN32
        DBG_PRINT( "GetLastError() returned: " << GetLastError() );
#endif

        // Set manager state.
        if( ++nErrorCount > INITERRORS_THRESHOLD )
        {
          // Maximum retries has been reached. Abandon reconnection.
          SetState( STATE_FATALERROR );
        }
        else
        {
          SetState( STATE_DISCONNECTED );

          // Wait 10 seconds before initiating next initialization.
          pal_sleep( 10000 );
        }
      }
    }

    // Sleep for 300 ms before the next status check.
    pal_sleep( 300 );
  }


  DBG_PRINT( "Communications manager service thread stopped." );
}


// --------------------------------------------------------------------------
// Function : WaitReady
//
// Description:
//   Will wait `ulWaitms' miliseconds until the Servent object signals the
//   ready state.
//   The ready state is usually signalled when the underlying IPC layer
//   is ready to connect to a peer.
//
// Arguments:
//   ulWaitms: long [IN], The number of miliseconds to wait until ready state
//                        is signalled.
//
// Return values:
//   true if ready state was signalled before the timeout.
//   false otherwise.
//
// --------------------------------------------------------------------------
bool CommunicationsManager::WaitReady( unsigned long ulWaitms )
{
  return m_Servent.WaitReady( ulWaitms );
}


// --------------------------------------------------------------------------
// Function : GetState
//
// Description:
//   Will return the current status of the communications manager.
//
// Arguments: (none)
//
// Return values:
//   The state of the communications manager.
//
// --------------------------------------------------------------------------
tManagerStatus CommunicationsManager::GetState( void ) const
{
  return m_eManagerStatus;
}


// --------------------------------------------------------------------------
// Function : SetState
//
// Description:
//   Will set the new manager status, and enable or disable the
//   MessageSender's queue.
//
// Arguments:
//   aState: manager status [IN], The new manager status.
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void CommunicationsManager::SetState( const tManagerStatus aState )
{
  m_eManagerStatus = aState;

  // Accept or reject new messages in the send queue.
  MessageSender::m_eSenderState = ( m_eManagerStatus == STATE_CONNECTED ) ? STATE_ENABLED : STATE_DISABLED;
}


// --------------------------------------------------------------------------
// Function : GetStatistics
//
// Description:
//   Will return the current statistics about this instance.
//
// Arguments:
//   aStats: MANAGER_STATISTICS [IN,OUT], An allocated struct that will
//           contain the stats.
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void CommunicationsManager::GetStatistics( MANAGER_STATISTICS* aStats )
{
  assert( aStats != NULL );

  aStats->nMsgQueued    = (unsigned int)m_SendQueue.size();
  aStats->nMsgSent      = m_nMsgSent;
  aStats->nMsgProcessed = m_nMsgProcessed;
  m_Servent.GetServentInfo( &(aStats->ServentInfo) );
}


// --------------------------------------------------------------------------
// Function : SenderThread::Work      [ THREAD ]
//
// Description:
//   This worker thread monitors the Communications Manager's send queue.
//   When a new message is available for sending, this thread will send it
//   to the other process, through the IPC layer.
//
// Arguments: (none)
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void SenderThread::Work( void )
{
  Message* pMsg;
  error_t  retCode;
  uint32_t nBytesSent;
  bool     bCanRead;


  DBG_PRINT( "Starting Sender thread" );

  while( !ShouldStop() )
  {
    // ---------------------------------------------------
    // Wait for a message to be posted to the send queue.
    // ---------------------------------------------------
    if( m_CommMgr->m_pSemaphore->WaitAndLock(25) != 0 )
    {
      // No message was available in 25 miliseconds.
      continue;
    }
    assert( m_CommMgr->m_SendQueue.size() > 0 );


    // ---------------------------------------------------------------
    // Check if we've got enough buffer to send a message.
    // This is done by verifying if there are no incoming messages.
    // * Doing otherwise would end up in a race condition between the
    //   sender and receiver threads.
    // This keeps the Servent receive buffer empty.
    // ---------------------------------------------------------------
    if( (retCode = m_CommMgr->m_Servent.CanRead( bCanRead )) != GOGOCM_UIS__NOERROR )
    {
      // An error occured.
      DBG_PRINT( "IO error. Failed CanRead(). Error code: 0x" << setbase(16)
           << retCode << setbase(10) );
#ifdef WIN32
      DBG_PRINT( "GetLastError() returned: " << GetLastError() );
#endif

      // Notify communications manager. Exit thread.
      m_CommMgr->SetState( STATE_DISCONNECTED );
      break;
    }
    if( bCanRead )
    {
      // Release lock on semaphore because we couldn't process queue message.
      m_CommMgr->m_pSemaphore->ReleaseLock();
      pal_sleep( 20 );    // Wait for message to be read by the receiver thread.
      continue;
    }


    // ----------------------------------------------
    // Retrieve message to be sent to the IPC layer.
    // ----------------------------------------------
    pMsg = m_CommMgr->m_SendQueue.front();
    assert( pMsg );


    // ---------------------------
    // Send message over the IPC.
    // ---------------------------
    retCode = m_CommMgr->m_Servent.WriteData( (void*)pMsg->rawdata, pMsg->GetRawSize(), nBytesSent );
    if( retCode != GOGOCM_UIS__NOERROR )
    {
      // An error occured.
      DBG_PRINT( "Error sending data. Error code: 0x" << setbase(16) << retCode );
#ifdef WIN32
      DBG_PRINT( "GetLastError() returned: " << setbase(10) << GetLastError() );
#endif

      // Notify communications manager. Exit thread.
      m_CommMgr->SetState( STATE_DISCONNECTED );
      break;
    }

    // --------------------------------------------
    // Verify if message has been sent completely.
    // --------------------------------------------
    if( nBytesSent != pMsg->GetRawSize() )
    {
      // ERROR:
      DBG_PRINT( "Failed to send all data. Sent " << nBytesSent << " bytes of data!" );

      // Notify communications manager. Exit thread.
      m_CommMgr->SetState( STATE_DISCONNECTED );
      break;
    }
    else
    {
      // Message has been sent successfully. Delete it.
      Message::FreeMessage( pMsg );

      // Remove message from send queue.
      m_CommMgr->m_SendQueue.pop();

      // Update counter.
      ++(m_CommMgr->m_nMsgSent);
    }

  }

  DBG_PRINT( "Exiting Sender thread" );
}


// --------------------------------------------------------------------------
// Function : ReceiverThread::Work      [ THREAD ]
//
// Description:
//   This worker thread monitors the IPC layer's Servent receive bufer.
//   If new data (messages) are available for reception, this thread will
//   extract and send them to the MessageProcessor for further processing.
//
// Arguments: (none)
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void ReceiverThread::Work(void)
{
  uint8_t   recvBuffer[64260];
  Message*  pMsg;
  uint32_t  nMessageLen;
  error_t   retCode;
  bool      bCanRead;


  DBG_PRINT( "Starting Receiver thread" );

  while( !ShouldStop() )
  {
    // ------------------------------------------------------------
    // Verify if there are messages to be read from the IPC layer.
    // ------------------------------------------------------------
    if( (retCode = m_CommMgr->m_Servent.CanRead( bCanRead )) != GOGOCM_UIS__NOERROR )
    {
      // An error occured.
      DBG_PRINT( "IO error. Failed CanRead(). Error code: 0x" << setbase(16)
           << retCode << setbase(10) );
#ifdef WIN32
      DBG_PRINT( "GetLastError() returned: " << GetLastError() );
#endif

      // Notify communications manager. Exit thread.
      m_CommMgr->SetState( STATE_DISCONNECTED );
      break;
    }
    if( !bCanRead )
    {
      pal_sleep( 25 );     // Wait for messages to be available.
      continue;
    }


    // ----------------------------------------------
    // Read the incoming message from the IPC layer.
    // ----------------------------------------------
    retCode = m_CommMgr->m_Servent.ReadData( (void*)recvBuffer, sizeof(recvBuffer), nMessageLen );
    if( retCode != GOGOCM_UIS__NOERROR )
    {
      // An error occured.
      DBG_PRINT( "Error sending data. Error code: 0x" << setbase(16) << retCode << setbase(10) );
#ifdef WIN32
      DBG_PRINT( "GetLastError() returned: " << GetLastError() );
#endif

      // Notify communications manager. Exit thread.
      m_CommMgr->SetState( STATE_DISCONNECTED );
      break;
    }


    // ------------------------------------------------
    // Process the message using the message receiver.
    // ------------------------------------------------
    pMsg = (Message*)recvBuffer;
    Message::AssertMessage( pMsg );
    m_CommMgr->m_MsgProcessor->ProcessMessage( pMsg );

    // Update the counter.
    ++(m_CommMgr->m_nMsgProcessed);
  }

  DBG_PRINT( "Exiting Receiver thread" );
}


} // namespace
