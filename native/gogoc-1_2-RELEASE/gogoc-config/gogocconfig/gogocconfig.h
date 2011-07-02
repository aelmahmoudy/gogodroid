// **************************************************************************
// $Id: gogocconfig.h,v 1.2 2010/02/08 21:40:06 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
//
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Wraps the NAME=VALUE configuration object to offer
//   gogoCLIENT-oriented configuration data accessors(get/set), along
//   with validation routines and error handling.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocconfig_gogocconfig_h__
#define __gogocconfig_gogocconfig_h__


#include <gogocconfig/namevalueconfig.h>
#include <gogocconfig/gogocuistrings.h>
#include <vector>
using namespace std;


namespace gogocconfig
{
  // Type definitions.
  typedef vector<error_t> t_errorarray;


  // ------------------------------------------------------------------------
  class GOGOCConfig
  {
  private:
    NameValueConfig*  m_pConfig;            // The configuration accessor object.
    string            m_sDfltCfgFile;       // The default config file name (for LoadDefault()).
    t_errorarray      m_lsValidationErrors; // The list of validation errors(for ValidateConfigData()).
    bool              m_bValid;             // A boolean indicating if data can successfully be retrieved or set.

  public:
    // Construction / destruction.
                      GOGOCConfig          ( void );
    virtual           ~GOGOCConfig         ( void );

    // Initialization.
    void              Initialize          ( const string& aConfigFile,
                                            const t_accessMode aEAccessMode,
                                            const string& aDfltCfgFile = "" );

    // Load / Cancel / Save / Load defaults.
    bool              Load                ( void );
    bool              Save                ( void );
    bool              CancelChanges       ( void );
    bool              LoadDefaults        ( void );

  protected:
    // Validation and error retrieval routines
    bool              ValidateConfigData  ( void ); // Used by ::Load()
  public:
    bool              IsConfigurationValid( void ) { return m_bValid; }
    const t_errorarray& GetValidationErrors ( void ) { return m_lsValidationErrors; }

  public:
    // gogoCLIENT Configuration Data accessors.
    void              Get_UserID          ( string& sUserID ) const;
    void              Set_UserID          ( const string& sUserID );

    void              Get_Passwd          ( string& sPasswd ) const;
    void              Set_Passwd          ( const string& sPasswd );

    void              Get_Server          ( string& sServer ) const;
    void              Set_Server          ( const string& sServer );

    void              Get_HostType        ( string& sHostType ) const;
    void              Set_HostType        ( const string& sHostType );

    void              Get_PrefixLen       ( string& sPrefixLen ) const;
    void              Set_PrefixLen       ( const string& sPrefixLen );

    void              Get_IfPrefix        ( string& sIfPrefix ) const;
    void              Set_IfPrefix        ( const string& sIfPrefix );

    void              Get_DnsServer       ( string& sDnsServer ) const;
    void              Set_DnsServer       ( const string& sDnsServer );

    void              Get_gogocDir         ( string& sgogocDir ) const;
    void              Set_gogocDir         ( const string& sgogocDir );

    void              Get_AuthMethod      ( string& sAuthMethod ) const;
    void              Set_AuthMethod      ( const string& sAuthMethod );

    void              Get_AutoRetryConnect( string& sAutoRetryConnect ) const;
    void              Set_AutoRetryConnect( const string& sAutoRetryConnect );

    void              Get_RetryDelay      ( string& sRetryDelay ) const;
    void              Set_RetryDelay      ( const string& sRetryDelay );

    void              Get_RetryDelayMax   ( string& sRetryDelayMax ) const;
    void              Set_RetryDelayMax   ( const string& sRetryDelayMax );

    void              Get_KeepAlive       ( string& sKeepAlive ) const;
    void              Set_KeepAlive       ( const string& sKeepAlive );

    void              Get_KeepAliveInterval( string& sKeepAliveInterval ) const;
    void              Set_KeepAliveInterval( const string& sKeepAliveInterval );

    void              Get_TunnelMode      ( string& sTunnelMode ) const;
    void              Set_TunnelMode      ( const string& sTunnelMode );

    void              Get_IfTunV6V4       ( string& sIfTunV6V4 ) const;
    void              Set_IfTunV6V4       ( const string& sIfTunV6V4 );

    void              Get_IfTunV6UDPV4    ( string& sIfTunV6UDPV4 ) const;
    void              Set_IfTunV6UDPV4    ( const string& sIfTunV6UDPV4 );

    void              Get_IfTunV4V6       ( string& sIfTunV4V6 ) const;
    void              Set_IfTunV4V6       ( const string& sIfTunV4V6 );

    void              Get_ClientV4        ( string& sClientV4 ) const;
    void              Set_ClientV4        ( const string& sClientV4 );

    void              Get_ClientV6        ( string& sClientV6 ) const;
    void              Set_ClientV6        ( const string& sClientV6 );

    void              Get_Template        ( string& sTemplate ) const;
    void              Set_Template        ( const string& sTemplate );

    void              Get_ProxyClient     ( string& sProxyClient ) const;
    void              Set_ProxyClient     ( const string& sProxyClient );

    void              Get_BrokerLstFile   ( string& sBrokerLstFile ) const;
    void              Set_BrokerLstFile   ( const string& sBrokerLstFile );

    void              Get_LastServFile    ( string& sLastServFile ) const;
    void              Set_LastServFile    ( const string& sLastServFile );

    void              Get_AlwaysUseLastSrv( string& sAlwaysUseLastSrv ) const;
    void              Set_AlwaysUseLastSrv( const string& sAlwaysUseLastSrv );

    void              Set_Log             ( const string& sLogDevice, const string& sLogLevel );
    void              Get_Log             ( const string& sLogDevice, string& sLogLevel ) const;

    void              Get_LogFileName     ( string& sLogFileName ) const;
    void              Set_LogFileName     ( const string& sLogFileName );

    void              Get_LogRotation     ( string& sLogRotation ) const;
    void              Set_LogRotation     ( const string& sLogRotation );

    void              Get_LogRotationSz   ( string& sLogRotationSz ) const;
    void              Set_LogRotationSz   ( const string& sLogRotationSz );

    void              Get_LogRotationDel  ( string& sLogRotationDel ) const;
    void              Set_LogRotationDel  ( const string& sLogRotationDel );

    void              Get_SysLogFacility  ( string& sSysLogFacility ) const;
    void              Set_SysLogFacility  ( const string& sSysLogFacility );

    void              Get_haccessProxyEnabled( string& shaccessProxyEnabled ) const;
    void              Set_haccessProxyEnabled( const string& shaccessProxyEnabled );

    void              Get_haccessWebEnabled  ( string& shaccessWebEnabled ) const;
    void              Set_haccessWebEnabled  ( const string& shaccessWebEnabled );

    void              Get_haccessDocumentRoot( string& shaccessDocumentRoot ) const;
    void              Set_haccessDocumentRoot( const string& shaccessDocumentRoot );

    void              Get_DSLite_Server   ( string& sDSLiteServer ) const;
    void              Set_DSLite_Server   ( const string& sDSLiteServer );

    void              Get_DSLite_Client   ( string& sDSLiteClient ) const;
    void              Set_DSLite_Client   ( const string& sDSLiteClient );
  };

}

#endif
