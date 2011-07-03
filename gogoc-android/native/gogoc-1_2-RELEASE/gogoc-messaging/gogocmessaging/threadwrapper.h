// **************************************************************************
// $Id: threadwrapper.h,v 1.1 2009/11/20 16:34:54 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Thread wrapper for Windows & other platforms.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_threadwrapper_h__
#define __gogocmessaging_threadwrapper_h__


#include <pal.h>

typedef ptr_t *  thread_t;
typedef ptr_t *  event_t;

#ifndef WINAPI
#define WINAPI
#endif

namespace gogocmessaging
{
  // ------------------------------------------------------------------------
  class ThreadWrapper
  {
  protected:
    thread_t        m_hThread;          // Thread handle.
    event_t         m_hQuitEvent;       // Handle for quit event.

    // Construction / destruction.
  protected:
                    ThreadWrapper       ( void );
  public:
    virtual         ~ThreadWrapper      ( void );

    // Start and stop the thread.
    bool            Run                 ( void );
    bool            Stop                ( void );

  protected:
    // The work function to be called by Run().
    virtual void    Work                ( void )=0;
    bool            ShouldStop          ( void ) const;

  private:
    static uint32_t WINAPI ThreadProc   ( void* lpvParam );
  };

}

#endif
