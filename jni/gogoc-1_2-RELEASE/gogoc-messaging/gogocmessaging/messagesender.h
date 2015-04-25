// **************************************************************************
// $Id: messagesender.h,v 1.1 2009/11/20 16:34:53 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
//
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   This component contains a way of posting messages. The messages are
//   accumulated into a send queue.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_messagesender_h__
#define __gogocmessaging_messagesender_h__


#include <gogocmessaging/message.h>
#include <gogocmessaging/semaphore.h>
#include <queue>
using namespace std;


namespace gogocmessaging
{
  // ------------------------------------------------------------------------
  class MessageSender
  {
  public:
    // Type definition.
    typedef enum { STATE_DISABLED, STATE_ENABLED } tSenderState;

  protected:
    tSenderState    m_eSenderState;
    std::queue<Message*> m_SendQueue;
    Semaphore*      m_pSemaphore; // Semaphore on queue.


  protected:
    // Construction / destruction.
                    MessageSender         ( void );
  public:
    virtual         ~MessageSender        ( void );

    // Offers a means of posting messages.
    void            PostMessage           ( Message* pMsg );

  protected:
    void            Reset                 ( void );
  };

}

#endif
