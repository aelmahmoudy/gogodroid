// **************************************************************************
// $Id: messagesender.cc,v 1.1 2009/11/20 16:34:56 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Implementation of the MessageSender class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gogocmessaging/messagesender.h>
#include <assert.h>


#define MAX_QUEUE_ITEMS     512


namespace gogocmessaging
{
// --------------------------------------------------------------------------
// Function : MessageSender constructor
//
// Description:
//   Will initialize a new MessageSender object.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
MessageSender::MessageSender( void ) :
  m_eSenderState( STATE_DISABLED ),
  m_pSemaphore(NULL)
{
  assert( m_SendQueue.empty() );

  // Create a semaphore.
  m_pSemaphore = new Semaphore( MAX_QUEUE_ITEMS, 0 );
  assert( m_pSemaphore != NULL );
}


// --------------------------------------------------------------------------
// Function : MessageSender destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
MessageSender::~MessageSender( void )
{
  if( m_pSemaphore != NULL )
  {
    delete m_pSemaphore;
    m_pSemaphore = NULL;
  }
}


// --------------------------------------------------------------------------
// Function : PostMessage
//
// Description:
//   Will put the message in the send queue for further processing, only if 
//   the state is enabled.
//
// Arguments:
//   pMsg: Message* [IN], The message to post.
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void MessageSender::PostMessage( Message* pMsg )
{
  if( m_eSenderState == STATE_ENABLED  &&  m_SendQueue.size() < MAX_QUEUE_ITEMS )
  {
    m_SendQueue.push( pMsg );
    m_pSemaphore->ReleaseLock();      // Increase semaphore count.
  }
}


// --------------------------------------------------------------------------
// Function : Reset
//
// Description:
//   Resets the MessageSender. Empties send queue and re-initializes the
//   semaphore object.
//
// Arguments:
//   pMsg: Message* [IN], The message to post.
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void MessageSender::Reset( void )
{
  // Empty queue
  while( !m_SendQueue.empty() )
    m_SendQueue.pop();

  // Reset semaphore object
  if( m_pSemaphore != NULL )
    delete m_pSemaphore;

  m_pSemaphore = new Semaphore( MAX_QUEUE_ITEMS, 0 );
}

}
