// **************************************************************************
// $Id: gogoc_c_wrapper.cc,v 1.2 2010/02/08 21:40:07 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
//
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Wraps the gogoCLIENT Configuration subsystem to offer C access.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include "pal.h"
#include <assert.h>

#include <gogocconfig/gogoc_c_wrapper.h>
#include <gogocconfig/gogocconfig.h>
#include <gogocconfig/gogocuistrings.h>
using namespace gogocconfig;


// This macro will try{} an operation and clear the value if an error occurs.
#define TRY_OR_CLEAR(X)         \
  try {                         \
    X;                          \
  } catch(...) {                \
    sValue="";                  \
  }


// The instance holder for the gogoCLIENT Configuration object.
static GOGOCConfig* gpConfig = NULL;


// --------------------------------------------------------------------------
// Function : initialize
//
// Description:
//   Will create a gogoCLIENT configuration object an load the configuration
//   data.
//
// Arguments:
//   szFileName: char* [IN], The name of the configuration file to load.
//
// Return values:
//   * 0 on successful initialization,
//   * -1 means configuration error; use get_config_errors,
//   * any other positive value means severe error; use get_ui_string for
//     more details.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
extern "C" int initialize( const char* szFileName )
{
  int iRet = 0;

  // Check if already initialized.
  if( gpConfig != NULL )
    return iRet;

  try
  {
    // Create new instance, initialize...
    gpConfig = new GOGOCConfig();
    gpConfig->Initialize( szFileName, AM_RW );

    // ...and load configuration data.
    iRet = gpConfig->Load() ? 0 : -1;
  }
  catch( error_t nErr )
  {
    iRet = nErr;
  }

  return iRet;
}

// --------------------------------------------------------------------------
// Function : un_initialize
//
// Description:
//   Will clean-up the gogoCLIENT configuration object.
//
// Arguments: (none)
//
// Return values: (none)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
extern "C" void un_initialize( void )
{
  if( gpConfig != NULL )
  {
    // Delete instance.
    delete gpConfig;
    gpConfig = NULL;
  }
}

// --------------------------------------------------------------------------
// Function : save_config
//
// Description:
//   saves the configuration
//
// Arguments:
//   void
//
// Return values:
//   * 0 on successful save,
//   * -1 on error on save,
//   * any other positive value means severe error; use get_ui_string for
//     more details.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
extern "C" int save_config( void )
{
  int iRet = 0;

  try
  {
  if( gpConfig != NULL)
  {
    // Saves the configuration
    iRet = gpConfig->Save() ? 0 : -1;
  }
  }
  catch (error_t nErr )
  {
    iRet = nErr;
  }

  return iRet;
}

// --------------------------------------------------------------------------
// Function : get_config_errors
//
// Description:
//   Will retrieve the errors from the gogoCLIENT configuration data.
//
// Arguments:
//   errc: int* [OUT], Will contain the number of errors in the array.
//   errv: int*[] [OUT], Will contain the error strings.
//
// Return values: (none)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
extern "C" void get_config_errors( int* errc, unsigned int* errv[] )
{
  assert( gpConfig != NULL );
  assert( *errv == NULL );
  const t_errorarray& vErr = gpConfig->GetValidationErrors();
  int i;

  // Get the number of items in the vector
  *errc = (int)vErr.size();

  if( *errc > 0 )
  {
    *errv = (unsigned int*)malloc((*errc) * sizeof(unsigned int) );

    // Copy the strings to the char* array
    for(i=0; i<*errc; i++)
      (*errv)[i] = vErr[i];
  }
}


// --------------------------------------------------------------------------
/* *************************************************************************/

/*            gogoCLIENT CONFIGURATION DATA WRAPPERS                       */

/* *************************************************************************/
// --------------------------------------------------------------------------


// --------------------------------------------------------------------------
extern "C" void get_user_id( char** szUserID )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szUserID == NULL );

  TRY_OR_CLEAR( gpConfig->Get_UserID( sValue ) );
  *szUserID = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void set_user_id( char* szUserID )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szUserID == NULL );

  sValue = szUserID;
  TRY_OR_CLEAR( gpConfig->Set_UserID( sValue ) );
}

// --------------------------------------------------------------------------
extern "C" void get_passwd( char** szPasswd )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szPasswd == NULL );

  TRY_OR_CLEAR( gpConfig->Get_Passwd( sValue ) );
  *szPasswd = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void set_passwd( char* szUserID )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szUserID == NULL );

  sValue = szUserID;
  TRY_OR_CLEAR( gpConfig->Set_Passwd( sValue ) );
}

// --------------------------------------------------------------------------
extern "C" void get_server( char** szServer )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szServer == NULL );

  TRY_OR_CLEAR( gpConfig->Get_Server( sValue ) );
  *szServer = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void set_server( char* szUserID )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szUserID == NULL );

  sValue = szUserID;
  TRY_OR_CLEAR( gpConfig->Set_Server( sValue ) );
}

// --------------------------------------------------------------------------
extern "C" void get_host_type( char** szHostType )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szHostType == NULL );

  TRY_OR_CLEAR( gpConfig->Get_HostType( sValue ) );
  *szHostType = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_prefixlen( int* piPrefixLen )
{
  string sValue;
  assert( gpConfig != NULL );

  TRY_OR_CLEAR( gpConfig->Get_PrefixLen( sValue ) );
  *piPrefixLen = (int)strtol(sValue.c_str(), (char**)NULL, 10);
}

// --------------------------------------------------------------------------
extern "C" void get_ifprefix( char** szIfPrefix )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szIfPrefix == NULL );

  TRY_OR_CLEAR( gpConfig->Get_IfPrefix( sValue ) );
  *szIfPrefix = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_dns_server( char** szDnsServer )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szDnsServer == NULL );

  TRY_OR_CLEAR( gpConfig->Get_DnsServer( sValue ) );
  *szDnsServer = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_gogoc_dir( char** szgogocDir )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szgogocDir == NULL );

  TRY_OR_CLEAR( gpConfig->Get_gogocDir( sValue ) );
  *szgogocDir = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_auth_method( char** szAuthMethod )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szAuthMethod == NULL );

  TRY_OR_CLEAR( gpConfig->Get_AuthMethod( sValue ) );
  *szAuthMethod = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_auto_retry_connect( tBoolean* pbAutoRetryConnect )
{
  string sValue;
  assert( gpConfig != NULL );

  TRY_OR_CLEAR( gpConfig->Get_AutoRetryConnect( sValue ) );
  *pbAutoRetryConnect = (tBoolean)(( pal_strcasecmp( sValue.c_str(), "yes" ) == 0 ) ? TRUE : FALSE);
}

// --------------------------------------------------------------------------
extern "C" void get_retry_delay( int* piRetryDelay )
{
  string sValue;
  assert( gpConfig != NULL );

  TRY_OR_CLEAR( gpConfig->Get_RetryDelay( sValue ) );
  *piRetryDelay = (int)strtol(sValue.c_str(), (char**)NULL, 10);
}

// --------------------------------------------------------------------------
extern "C" void get_retry_delay_max( int* piRetryDelayMax )
{
  string sValue;
  assert( gpConfig != NULL );

  TRY_OR_CLEAR( gpConfig->Get_RetryDelayMax( sValue ) );
  *piRetryDelayMax = (int)strtol(sValue.c_str(), (char**)NULL, 10);
}

// --------------------------------------------------------------------------
extern "C" void get_keepalive( tBoolean* pbKeepAlive )
{
  string sValue;
  assert( gpConfig != NULL );

  TRY_OR_CLEAR( gpConfig->Get_KeepAlive( sValue ) );
  *pbKeepAlive = (tBoolean)(( pal_strcasecmp( sValue.c_str(), "yes" ) == 0 ) ? TRUE : FALSE);
}

// --------------------------------------------------------------------------
extern "C" void get_keepalive_interval( int* piKaInterval )
{
  string sValue;
  assert( gpConfig != NULL );

  TRY_OR_CLEAR( gpConfig->Get_KeepAliveInterval( sValue ) );
  *piKaInterval = (int)strtol(sValue.c_str(), (char**)NULL, 10);
}

// --------------------------------------------------------------------------
extern "C" void get_tunnel_mode( char** szTunMode )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szTunMode == NULL );

  TRY_OR_CLEAR( gpConfig->Get_TunnelMode( sValue ) );
  *szTunMode = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_if_tun_v6v4( char** szIf )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szIf == NULL );

  TRY_OR_CLEAR( gpConfig->Get_IfTunV6V4( sValue ) );
  *szIf = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_if_tun_v6udpv4( char** szIf )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szIf == NULL );

  TRY_OR_CLEAR( gpConfig->Get_IfTunV6UDPV4( sValue ) );
  *szIf = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_if_tun_v4v6( char** szIf )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szIf == NULL );

  TRY_OR_CLEAR( gpConfig->Get_IfTunV4V6( sValue ) );
  *szIf = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_client_v4( char** szClientV4 )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szClientV4 == NULL );

  TRY_OR_CLEAR( gpConfig->Get_ClientV4( sValue ) );
  *szClientV4 = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_dslite_server( char** szDSLite_Server )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szDSLite_Server == NULL );

  TRY_OR_CLEAR( gpConfig->Get_DSLite_Server( sValue ) );
  *szDSLite_Server = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_dslite_client( char** szDSLite_Client )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szDSLite_Client == NULL );

  TRY_OR_CLEAR( gpConfig->Get_DSLite_Client( sValue ) );
  *szDSLite_Client = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_client_v6( char** szClientV6 )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szClientV6 == NULL );

  TRY_OR_CLEAR( gpConfig->Get_ClientV6( sValue ) );
  *szClientV6 = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_template( char** szTemplate )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szTemplate == NULL );

  TRY_OR_CLEAR( gpConfig->Get_Template( sValue ) );
  *szTemplate = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_proxy_client( tBoolean* pbProxyClient )
{
  string sValue;
  assert( gpConfig != NULL );

  TRY_OR_CLEAR( gpConfig->Get_ProxyClient( sValue ) );
  *pbProxyClient = (tBoolean)(( pal_strcasecmp( sValue.c_str(), "yes" ) == 0 ) ? TRUE : FALSE);
}

// --------------------------------------------------------------------------
extern "C" void get_broker_list_file( char** szBrokerFile )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szBrokerFile == NULL );

  TRY_OR_CLEAR( gpConfig->Get_BrokerLstFile( sValue ) );
  *szBrokerFile = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_last_server_file( char** szLastServFile )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szLastServFile == NULL );

  TRY_OR_CLEAR( gpConfig->Get_LastServFile( sValue ) );
  *szLastServFile = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_always_use_last_server( tBoolean* pbUseLastServ )
{
  string sValue;
  assert( gpConfig != NULL );

  TRY_OR_CLEAR( gpConfig->Get_AlwaysUseLastSrv( sValue ) );
  *pbUseLastServ = (tBoolean)(( pal_strcasecmp( sValue.c_str(), "yes" ) == 0 ) ? TRUE : FALSE);
}

// --------------------------------------------------------------------------
extern "C" void get_log( const char* szLogDevice, short* psLevel )
{
  string sValue;
  assert( gpConfig != NULL );

  TRY_OR_CLEAR( gpConfig->Get_Log( szLogDevice, sValue ) );
  *psLevel = (short)strtol(sValue.c_str(), (char**)NULL, 10);
}

// --------------------------------------------------------------------------
extern "C" void get_log_filename( char** szLogFilename )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szLogFilename == NULL );

  TRY_OR_CLEAR( gpConfig->Get_LogFileName( sValue ) );
  *szLogFilename = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_log_rotation( tBoolean* pbLogRotation )
{
  string sValue;
  assert( gpConfig != NULL );

  TRY_OR_CLEAR( gpConfig->Get_LogRotation( sValue ) );
  *pbLogRotation = (tBoolean)(( pal_strcasecmp( sValue.c_str(), "yes" ) == 0 ) ? TRUE : FALSE);
}

// --------------------------------------------------------------------------
extern "C" void get_log_rotation_sz( int* piLogRotSz )
{
  string sValue;
  assert( gpConfig != NULL );

  TRY_OR_CLEAR( gpConfig->Get_LogRotationSz( sValue ) );
  *piLogRotSz = (int)strtol(sValue.c_str(), (char**)NULL, 10);
}


// --------------------------------------------------------------------------
extern "C" void get_log_rotation_del( tBoolean* pbLogRotDel )
{
  string sValue;
  assert( gpConfig != NULL );

  TRY_OR_CLEAR( gpConfig->Get_LogRotationDel( sValue ) );
  *pbLogRotDel = ( pal_strcasecmp( sValue.c_str(), "yes" ) == 0 ) ? TRUE : FALSE;
}

// --------------------------------------------------------------------------
extern "C" void get_syslog_facility( char** szSyslog )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *szSyslog == NULL );

  TRY_OR_CLEAR( gpConfig->Get_SysLogFacility( sValue ) );
  *szSyslog = pal_strdup( sValue.c_str() );
}

// --------------------------------------------------------------------------
extern "C" void get_haccess_proxy_enabled( tBoolean* pbhaccessProxyEnabled )
{
  string sValue;
  assert( gpConfig != NULL );

  TRY_OR_CLEAR( gpConfig->Get_haccessProxyEnabled( sValue ) );
  *pbhaccessProxyEnabled = (tBoolean)(( pal_strcasecmp( sValue.c_str(), "yes" ) == 0 ) ? TRUE : FALSE);
}

// --------------------------------------------------------------------------
extern "C" void get_haccess_web_enabled( tBoolean* pbhaccessWebEnabled )
{
  string sValue;
  assert( gpConfig != NULL );

  TRY_OR_CLEAR( gpConfig->Get_haccessWebEnabled( sValue ) );
  *pbhaccessWebEnabled = (tBoolean)(( pal_strcasecmp( sValue.c_str(), "yes" ) == 0 ) ? TRUE : FALSE);
}

// --------------------------------------------------------------------------
extern "C" void get_haccess_document_root( char** pszhaccessDocRoot )
{
  string sValue;
  assert( gpConfig != NULL );
  assert( *pszhaccessDocRoot == NULL );

  TRY_OR_CLEAR( gpConfig->Get_haccessDocumentRoot( sValue ) );
  *pszhaccessDocRoot = pal_strdup( sValue.c_str() );
}
