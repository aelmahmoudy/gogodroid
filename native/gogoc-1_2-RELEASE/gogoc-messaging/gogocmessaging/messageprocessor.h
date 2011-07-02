// **************************************************************************
// $Id: messageprocessor.h,v 1.1 2009/11/20 16:34:53 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   This component provides a way of processing incoming messages. It is 
//   fully abstracted and forces the applications to implement the 
//   ProcessMessage function.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_messageprocessor_h__
#define __gogocmessaging_messageprocessor_h__


#include <gogocmessaging/message.h>
#include <gogocmessaging/gogocuistrings.h>


namespace gogocmessaging
{
  // ------------------------------------------------------------------------
  class MessageProcessor
  {
  public:
    // Type definition.
    typedef enum { STATE_DISABLED, STATE_ENABLED } tProcessorState;

  protected:
    tProcessorState m_eProcessorState;

  protected:
    // Construction / destruction.
                    MessageProcessor      ( void ) : m_eProcessorState(STATE_DISABLED) {};
  public:
    virtual         ~MessageProcessor     ( void ) {};

    // Message Processing.
    virtual error_t ProcessMessage        ( Message* pMsg )=0;
  };

}

#endif
