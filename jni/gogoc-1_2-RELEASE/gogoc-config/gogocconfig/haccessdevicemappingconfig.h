// **************************************************************************
// $Id: haccessdevicemappingconfig.h,v 1.1 2009/11/20 16:30:24 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Wraps the NAME=VALUE configuration object to offer HomeAccess Device Mapping
//   generic accessors.
//
// Author: Charles Nepveu
//
// Creation Date: February 2007
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocconfig_haccessdevicemappingconfig_h__
#define __gogocconfig_haccessdevicemappingconfig_h__


#include <gogocconfig/namevalueconfig.h>
#include <gogocconfig/gogocuistrings.h>
#include <vector>
#include <map>
using namespace std;


namespace gogocconfig
{
  // Type definition.
  typedef vector<string>            t_stringarray;
  typedef map<string, string> t_stringmap;


  // ------------------------------------------------------------------------
  class HACCESSDeviceMappingConfig
  {
  private:
    NameValueConfig*  m_pConfig;            // The configuration accessor object.

  public:
    // Construction / destruction.
                      HACCESSDeviceMappingConfig( void );
    virtual           ~HACCESSDeviceMappingConfig( void );  

    // Initialization.
    void              Initialize          ( const string& aConfigFile,
                                            const t_accessMode aEAccessMode );

    // Load / Cancel / Save.
    bool              Load                ( void );
    bool              Save                ( void );
    bool              CancelChanges       ( void );

    // Validation routine.
    bool              ValidateConfig      ( void );

    // HomeAccess Device Mapping Configuration accessors.
    bool              AddDeviceMapping    ( const string& aName, const string& aAddress );
    bool              DelDeviceMapping    ( const string& aName );

    bool              GetDeviceAddress    ( const string& aName, string& aAddress ) const;
    bool              SetDeviceAddress    ( const string& aName, const string& aAddress );

    bool              GetDeviceNameList   ( t_stringarray& aNameList ) const;
    bool              GetDeviceList       ( t_stringmap& aDeviceMap ) const;

  private:
    bool              ValidateDeviceName  ( const string& aDeviceName );
    bool              ValidateIPAddress   ( const string& aIPAddress );
  };

}

#endif
