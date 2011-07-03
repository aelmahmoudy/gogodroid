// **************************************************************************
// $Id: semaphore.cc,v 1.1 2009/11/20 16:34:57 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   POSIX Semaphore wrapper. Use -lrt link option.
//
// Author: Charles Nepveu
//
// Creation Date: December 2006
// __________________________________________________________________________
// **************************************************************************
#include <gogocmessaging/semaphore.h>
#include <assert.h>
#include <unistd.h>


namespace gogocmessaging
{
// --------------------------------------------------------------------------
// Function : Semaphore constructor
//
// Description:
//   Will initialize a new Semaphore object.
//
// Arguments:
//   nCount: int [IN], The initial count of semaphore
//
// Return values: (N/A)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
Semaphore::Semaphore( unsigned int nMaxCount, unsigned int nInitialCount )
{
  assert( nMaxCount >= nInitialCount );
  int i;

  i = sem_init( &m_Semaphore, 0, nMaxCount );
  assert( i == 0 );

  // Lock a certain count.
  for( i=nMaxCount-nInitialCount; i>0; i-- )
    WaitAndLock();
}


// --------------------------------------------------------------------------
// Function : Semaphore destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
Semaphore::~Semaphore( void )
{
  sem_destroy( &m_Semaphore );
}


// --------------------------------------------------------------------------
// Function : WaitAndLock
//
// Description:
//   Blocks execution until semaphore object is available.
//   Locks (decrements) semaphore count.
//
// Arguments:
//   ulWaitms: long [IN], The time to wait until state is signalled.
//                        If 0, the timeout is infinite.
//
// Return values:
//   0: Successfuly obtained lock on semaphore object
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
int Semaphore::WaitAndLock( unsigned long ulWaitms )
{
  int retCode = -1;

  if( ulWaitms > 0 )
  {
    unsigned long wait = 0;

    do
    {
      retCode = sem_trywait( &m_Semaphore );
      usleep( 25000 ); wait += 25;
    }
    while( retCode != 0  &&  wait < ulWaitms );
  }
  else
    retCode = sem_wait( &m_Semaphore );

  return retCode;
}


// --------------------------------------------------------------------------
// Function : ReleaseLock
//
// Description:
//   Releases lock on semaphore object (increments semaphore count).
//
// Arguments: (none)
//
// Return values:
//   0: Successfuly released lock on semaphore object.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
int Semaphore::ReleaseLock( void )
{
  return sem_post( &m_Semaphore );
}


} // namespace
