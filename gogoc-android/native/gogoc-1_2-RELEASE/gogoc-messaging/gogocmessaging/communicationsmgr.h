// **************************************************************************
// $Id: communicationsmgr.h,v 1.1 2009/11/20 16:34:52 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Definition the the Communications manager.
//   This is the main coordinator. The applications will instantiate one 
//   object of this type and register a message processor. Incoming data will
//   be monitored in a different thread. When incoming data is received from 
//   the IPC communications layer, the communications manager will 
//   deserialize it into a message and dispatch it using the MessageProcessor.
//   The communications manager will also (in a separate thread) extract 
//   messages from the MessageSender's queue, serialize them and send them 
//   to the IPC communications layer.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_communicationsmgr_h__
#define __gogocmessaging_communicationsmgr_h__


#include <gogocmessaging/messagesender.h>
#include <gogocmessaging/messageprocessor.h>
#include <gogocmessaging/servent.h>
#include <gogocmessaging/threadwrapper.h>
#include <gogocmessaging/gogocuistrings.h>


namespace gogocmessaging
{
  // Type definitions.
  typedef enum { CLIENT_MANAGER, SERVER_MANAGER } tManagerMode;
  typedef enum { STATE_DISCONNECTED, STATE_PENDINGCONNECTION, 
                 STATE_CONNECTED, STATE_FATALERROR } tManagerStatus;
  class CommunicationsManager;


  // ------------------------------------------------------------------------
  class SenderThread : public ThreadWrapper
  {
  private:
    CommunicationsManager* m_CommMgr;

  public:
    // Construction / destruction.
                    SenderThread          ( CommunicationsManager* aCommMgr ) : 
                                            ThreadWrapper(), m_CommMgr(aCommMgr) {};
    virtual         ~SenderThread         ( void ) {};

  protected:
    // Work method.
    void            Work                  ( void );
  };


  // ------------------------------------------------------------------------
  class ReceiverThread : public ThreadWrapper
  {
  private:
    CommunicationsManager* m_CommMgr;

  public:
    // Construction / destruction.
                    ReceiverThread        ( CommunicationsManager* aCommMgr ) : 
                                            ThreadWrapper(), m_CommMgr(aCommMgr) {};
    virtual         ~ReceiverThread       ( void ) {};

  protected:
    // Work method.
    void            Work                  ( void );
  };


  // ------------------------------------------------------------------------
  class CommunicationsManager : public MessageSender, public ThreadWrapper
  {
  public:
    // Type definitions:
    struct MANAGER_STATISTICS         // Information available.
    {
      unsigned int  nMsgQueued;
      unsigned int  nMsgSent;
      unsigned int  nMsgProcessed;
      SERVENT_INFO  ServentInfo;
    };

  private:
    IPCServent*     m_IPCServent;     // For IPC operations.
    Servent         m_Servent;        // For message sending / reception.
    MessageProcessor* m_MsgProcessor; // For message processing.
    ReceiverThread* m_ReceiverThread; // Monitors servent for incoming messages.
    SenderThread*   m_SenderThread;   // Monitors send queue and sends messages.
    tManagerMode    m_eManagerMode;   // Is this instance a Server or client?
    tManagerStatus  m_eManagerStatus; // The current state of the manager.
    unsigned int    m_nMsgSent;       // The number of messages sent until now.
    unsigned int    m_nMsgProcessed;  // The number of messages received until now.

  public:
    // Construction / destruction.
                    CommunicationsManager ( tManagerMode aEMode, MessageProcessor* aProcessor );
    virtual         ~CommunicationsManager( void );


    void            GetStatistics         ( MANAGER_STATISTICS* aStats );
    bool            WaitReady             ( unsigned long ulWaitms=750 );

    // Communications manager status accessors.
    tManagerStatus  GetState              ( void ) const;
  private:
    void            SetState              ( const tManagerStatus aState );

    // Worker thread function. Monitors service availability.
    void            Work                  ( void );

    // Instance initialization.
    error_t         Initialize            ( void );

    // Cleans up this instance
    void            _CleanupInstance      ( void );

    // Friends and family.
    friend class    SenderThread;
    friend class    ReceiverThread;
  };

}

#endif
