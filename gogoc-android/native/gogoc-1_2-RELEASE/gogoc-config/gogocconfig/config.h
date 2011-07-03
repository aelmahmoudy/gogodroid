// **************************************************************************
// $Id: config.h,v 1.1 2009/11/20 16:30:23 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Provides a simple configuration accessor. This class contains means of
//   applying and cancelling configuration changes.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocconfig_config_h__
#define __gogocconfig_config_h__


#include <gogocconfig/parser.h>


namespace gogocconfig
{
  // Type definitions.
  typedef enum { AM_READ, AM_CREATE, AM_RW } t_accessMode;


  // ------------------------------------------------------------------------
  class Config
  {
  protected:
    Parser*         m_pParser;            // Generic parser object.
    t_accessMode    m_eAccessMode;        // Access mode to configuration.
    string          m_sConfigFile;        // Configuration file name.

  protected:
    // Construction / destruction
                    Config                ( const string& aConfigFile,
                                            const t_accessMode aEAccessMode );
  public:
    virtual         ~Config               ( void );

    virtual bool    LoadConfiguration     ( void ); // Load config data
    virtual bool    ApplyConfiguration    ( void ); // Save changes
    virtual bool    CancelConfiguration   ( void ); // Revert changes

    virtual bool    OverrideConfiguration ( const string& aFileName );
  };

}

#endif
