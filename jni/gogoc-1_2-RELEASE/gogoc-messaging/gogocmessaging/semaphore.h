// **************************************************************************
// $Id: semaphore.h,v 1.1 2009/11/20 16:34:54 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Semaphore wrapper for Windows & other platforms.
//
// Author: Charles Nepveu
//
// Creation Date: December 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_semaphore_h__
#define __gogocmessaging_semaphore_h__

#include <pal.h>


#if defined(WIN32) || defined(WINCE)
typedef ptr_t*                sem_t;
#else
#include <semaphore.h>
#endif


namespace gogocmessaging
{
  // ------------------------------------------------------------------------
  class Semaphore
  {
  protected:
    sem_t           m_Semaphore;          // Semaphore *nix struct.

  public:
                    Semaphore             ( unsigned int nMaxCount=1, unsigned int nInitialCount=1 );
                    ~Semaphore            ( void );

    int             WaitAndLock           ( unsigned long ulWaitms=0 ); // Blocking
    int             ReleaseLock           ( void );
  };

}

#endif
