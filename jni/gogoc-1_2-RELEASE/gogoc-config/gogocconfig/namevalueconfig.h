// **************************************************************************
// $Id: namevalueconfig.h,v 1.1 2009/11/20 16:30:24 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   This class provides access to the NAME=VALUE configuration data.
//   Consultation and modification of configuration data is allowed.
//   This class extends the Config class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocconfig_namevalueconfig_h__
#define __gogocconfig_namevalueconfig_h__


#include <gogocconfig/config.h>
#include <list>
using namespace std;


namespace gogocconfig
{
  // Type definitions.
  typedef list<string> stringlist;


  // ------------------------------------------------------------------------
  class NameValueConfig : public Config
  {
  public:
    // Construction / destruction
                    NameValueConfig       ( const string& aFileName, const t_accessMode aEAccessMode );
    virtual         ~NameValueConfig      ( void );

    // NameValueConfig operations
    void            GetVariableValue      ( const string& aName, string& aValue ) const;
    void            SetVariableValue      ( const string& aName, const string& aValue );
    void            RemoveVariable        ( const string& aName );
    void            GetVariableNameList   ( stringlist& aList ) const;
  };

}

#endif
