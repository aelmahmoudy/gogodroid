// **************************************************************************
// $Id: servent.h,v 1.1 2009/11/20 16:34:54 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   This component will be used by the messaging layer to communicate 
//   through the IPC. Prior to use the Servent component, an application must
//   register a IPCServent-derived object. The Initialize method will 
//   initialize the IPCServent component. The SendData and ReceiveData 
//   methods are standard IO routines used to send and receive user data. 
//   The Servent will implement a way of fragmenting user data in several 
//   packets before sending them on the IPC medium. Also, it will be able to 
//   reconstitute the fragmented user data upon reception.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_servent_h__
#define __gogocmessaging_servent_h__


#include <gogocmessaging/ipcservent.h>
#include <gogocmessaging/semaphore.h>


namespace gogocmessaging
{
  typedef unsigned long long counter_t;

  // Type definitions.
  typedef struct __SERVENT_INFO
  {
    counter_t       nTtlBytesRead;
    counter_t       nTtlBytesWritten;
  } SERVENT_INFO, *PSERVENT_INFO;


  // ------------------------------------------------------------------------
  class Servent
  {
  private:
    IPCServent*     m_pIPCServent;      // IPC server/client connectivity.
    counter_t       m_nTtlBytesRead;    // Bytes received counter.
    counter_t       m_nTtlBytesWritten; // Bytes sent counter.
    Semaphore*      m_pSemIPCMutex;     // Mutex for IO operations on IPC.

  public:
    // Construction / destruction
                    Servent               ( void );
    virtual         ~Servent              ( void );

    // Initialization routine.
    error_t         Initialize            ( IPCServent* pIPCServent );
    bool            WaitReady             ( unsigned long ulWaitms );

    // IO operation routines.
    error_t         ReadData              ( void* pvReadBuffer, const uint32_t nBufferSize, uint32_t& nRead );
    error_t         WriteData             ( const void* pvData, const uint32_t nDataSize, uint32_t& nWritten );
    error_t         CanRead               ( bool& bCanRead );
    error_t         CanWrite              ( bool& bCanWrite );

    // Object Statistics info.
    void            GetServentInfo        ( PSERVENT_INFO pObj );

  private:
    error_t         _ReadData             ( void* pvReadBuffer, const uint32_t nBufferSize, uint32_t& nRead );
    error_t         _WriteData            ( const void* pvData, const uint32_t nDataSize, uint32_t& nWritten );
  };

}

#endif
