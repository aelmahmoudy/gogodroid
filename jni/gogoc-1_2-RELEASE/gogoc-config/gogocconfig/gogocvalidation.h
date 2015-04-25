// **************************************************************************
// $Id: gogocvalidation.h,v 1.2 2010/02/08 21:40:06 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
//
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Implements functions to validate contents of gogoCLIENT
//   configuration data.
//   When errors are found in the data, the error message relative to the
//   error can be retrieved with the GetLastError() function.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocconfig_gogocvalidation_h__
#define __gogocconfig_gogocvalidation_h__


#include <gogocconfig/gogocuistrings.h>
#include <string>
using namespace std;


// These DEFINEs are used in other modules as well.
#define STR_YES                     "yes"
#define STR_NO                      "no"
#define STR_ANONYMOUS               "anonymous"
#define STR_PLAIN                   "plain"
#define STR_DIGESTMD5               "digest-md5"
#define STR_PASSDSS3DES1            "passdss-3des-1"
#define STR_V6V4                    "v6v4"
#define STR_V6ANYV4                 "v6anyv4"
#define STR_V6UDPV4                 "v6udpv4"
#define STR_V4V6                    "v4v6"
#define STR_DSLITE                  "dslite"
#define STR_ANY                     "any"
#define STR_AUTO                    "auto"
#define STR_LOGDEV_CONSOLE          "console"
#define STR_LOGDEV_STDERR           "stderr"
#define STR_LOGDEV_FILE             "file"
#define STR_LOGDEV_SYSLOG           "syslog"
#define STR_HOSTTYPE_HOST           "host"
#define STR_HOSTTYPE_ROUTER         "router"
#define STR_TEMPL_WINDOWS           "windows"
#define STR_LOGROTSZ_16K            "16"
#define STR_LOGROTSZ_32K            "32"
#define STR_LOGROTSZ_128K           "128"
#define STR_LOGROTSZ_1024K          "1024"


namespace gogocconfig
{
  // Error retrieval.
  const error_t GetLastError( void );

  // Validation routines.
  bool Validate_UserID          ( const string& sUserID );

  bool Validate_Passwd          ( const string& sPasswd );

  bool Validate_Server          ( const string& sServer );

  bool Validate_HostType        ( const string& sHostType );

  bool Validate_PrefixLen       ( const string& sPrefixLen );

  bool Validate_IfPrefix        ( const string& sIfPrefix );

  bool Validate_DnsServer       ( const string& sDnsServer );

  bool Validate_gogocDir         ( const string& sgogocDir );

  bool Validate_AuthMethod      ( const string& sAuthMethod );

  bool Validate_AutoRetryConnect( const string& sAutoRetryConnect );

  bool Validate_RetryDelay      ( const string& sRetryDelay );

  bool Validate_RetryDelayMax   ( const string& sRetryDelayMax );

  bool Validate_KeepAlive       ( const string& sKeepAlive );

  bool Validate_KeepAliveInterval( const string& sKeepAliveInterval );

  bool Validate_TunnelMode      ( const string& sTunnelMode );

  bool Validate_IfTunV6V4       ( const string& sIfTunV6V4 );

  bool Validate_IfTunV6UDPV4    ( const string& sIfTunV6UDPV4 );

  bool Validate_IfTunV4V6       ( const string& sIfTunV4V6 );

  bool Validate_ClientV4        ( const string& sClientV4 );

  bool Validate_ClientV6        ( const string& sClientV6 );

  bool Validate_Template        ( const string& sTemplate );

  bool Validate_ProxyClient     ( const string& sProxyClient );

  bool Validate_BrokerLstFile   ( const string& sBrokerLstFile );

  bool Validate_LastServFile    ( const string& sLastServFile );

  bool Validate_AlwaysUseLastSrv( const string& sAlwaysUseLastSrv );

  bool Validate_LogDevice       ( const string& sLogDevice );
  bool Validate_LogLevel        ( const string& sLogLevel );

  bool Validate_LogFileName     ( const string& sLogFileName );

  bool Validate_LogRotation     ( const string& sLogRotation );

  bool Validate_LogRotationSz   ( const string& sLogRotationSz );

  bool Validate_LogRotationDel  ( const string& sLogRotationDel );

  bool Validate_SysLogFacility  ( const string& sSysLogFacility );

  bool Validate_haccessProxyEnabled( const string& shaccessProxyEnabled );

  bool Validate_haccessWebEnabled  ( const string& shaccessWebEnabled );

  bool Validate_haccessDocumentRoot( const string& shaccessDocumentRoot );

  bool Validate_DSLite          ( const string& sDSLite );
}

#endif
