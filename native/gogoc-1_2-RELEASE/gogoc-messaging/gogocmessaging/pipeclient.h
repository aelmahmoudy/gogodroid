// **************************************************************************
// $Id: pipeclient.h,v 1.1 2009/11/20 16:34:53 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Defines a specialized IPC Client: The Named pipe client.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_pipeclient_h__
#define __gogocmessaging_pipeclient_h__


#include <gogocmessaging/ipcclient.h>
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
  class PipeClient : public IPCClient, public PipeIO
  {
  private:
    IPC_HANDLE      m_PipeHandle;
    string          m_PipeName;

  public:
    // Construction / destruction
                    PipeClient            ( const string& aPipeName );
    virtual         ~PipeClient           ( void );

    // IPC Client overrides.
    virtual error_t Connect               ( void );   // Blocking
    virtual error_t Disconnect            ( void );
  };

}
#if defined(WIN32) || defined(WINCE)
#pragma warning( default:4250 )
#endif

#endif
