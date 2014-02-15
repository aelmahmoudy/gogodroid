// **************************************************************************
// $Id: ipcclient.h,v 1.1 2009/11/20 16:34:53 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Defines a generic IPC Client.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_ipcclient_h__
#define __gogocmessaging_ipcclient_h__


#include <gogocmessaging/ipcservent.h>


namespace gogocmessaging
{
  // ------------------------------------------------------------------------
  class IPCClient : virtual public IPCServent
  {
  protected:
    // Construction / destruction
                    IPCClient             ( void );
  public:
    virtual         ~IPCClient            ( void );

    // IPC Servent overrides.
    virtual error_t Initialize            ( void );   // Blocking
    virtual error_t UnInitialize          ( void );   // Blocking
    virtual bool    WaitReady             ( unsigned long ulWaitms );

    // IPC Client operations.
    virtual error_t Connect               ( void ) = 0;   // Blocking
    virtual error_t Disconnect            ( void ) = 0;
  };

}

#endif
