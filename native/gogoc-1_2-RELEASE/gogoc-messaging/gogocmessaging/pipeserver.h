// **************************************************************************
// $Id: pipeserver.h,v 1.1 2009/11/20 16:34:54 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Defines a specialized IPC Server: The Named pipe server.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_pipeserver_h__
#define __gogocmessaging_pipeserver_h__


#include <gogocmessaging/ipcserver.h>
#include <gogocmessaging/pipeio.h>
#include <gogocmessaging/gogocuistrings.h>
#include <string>
using namespace std;


#if defined(WIN32) || defined(WINCE)
#pragma warning( disable:4250 )
#endif
namespace gogocmessaging
{
  // ------------------------------------------------------------------------
  class PipeServer : public IPCServer, public PipeIO
  {
  private:
    string          m_PipeName;
    bool            m_bClientConnected;

  public:
    // Construction / destruction
                    PipeServer            ( const string& aPipeName );
    virtual         ~PipeServer           ( void );

    // IPC Server overrides.
    virtual error_t CreateConnectionPoint ( void );   // Non-Blocking
    virtual error_t AcceptConnection      ( void );   // Blocking
    virtual error_t CloseConnection       ( void );   // Non-Blocking
  };

}
#if defined(WIN32) || defined(WINCE)
#pragma warning( default:4250 )
#endif

#endif
