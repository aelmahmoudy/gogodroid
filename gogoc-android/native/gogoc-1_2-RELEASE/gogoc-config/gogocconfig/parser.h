// **************************************************************************
// $Id: parser.h,v 1.1 2009/11/20 16:30:25 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Provides a generic means of parsing a file.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocconfig_parser_h__
#define __gogocconfig_parser_h__


#include <string>
using namespace std;


namespace gogocconfig
{
  // ------------------------------------------------------------------------
  class Parser
  {
  protected:
    // Construction / destruction
                    Parser                ( void ) {;};
  public:
    virtual         ~Parser               ( void ) {;};

  public:
    // Pure abstract
    virtual bool    ReadConfigurationData ( const string& aFilename ) = 0;
    virtual bool    WriteConfigurationData( const string& aFilename ) = 0;
  };

}

#endif
