/* *********************************************************************** */
/* $Id: gogocuistrings.c,v 1.2 2010/03/07 16:21:53 carl Exp $            */
/*                                                                         */
/* Copyright (c) 2007 gogo6 Inc. All rights reserved.                     */
/*                                                                         */
/*   For license information refer to CLIENT-LICENSE.TXT                   */
/*                                                                         */
/* Description:                                                            */
/*   Offers default UI string for errors and other.                        */
/*                                                                         */
/* You may translate the strings herein as you wish.                       */
/*                                                                         */
/* Author: Charles Nepveu                                                  */
/*                                                                         */
/* Creation Date: November 2006                                            */
/* _______________________________________________________________________ */
/* *********************************************************************** */
#include <gogocconfig/gogocuistrings.h>


/* Struct containing string IDs with the related string.                   */
typedef struct { error_t _id; const char* _str; } tgogocUIStrings;


static const tgogocUIStrings gogocUIStrings[] = {

  /* And to start with... The NameValueParser strings */
  { GOGOC_UIS__GROUP_NAMEVALUEPARSER,    // title
    "Name=Value Parser Error" },
  { GOGOC_UIS__NMP_OPENFAIL,
    "Failed to open specified file." },
  { GOGOC_UIS__NMP_BADCONFIGFILE,
    "Bad configuration file." },
  { GOGOC_UIS__NMP_OPENFAILWRITE,
    "Failed to open file to write configuration data." },

  /* Next... Config strings */
  { GOGOC_UIS__GROUP_GENERICCONFIG,    // title
    "Generic Configuration Error" },
  { GOGOC_UIS__CFG_CANNOTLOADWHENCREATE,
    "Cannot load configuration when access mode is CREATE." },
  { GOGOC_UIS__CFG_CANNOTAPPLYWHENREAD,
    "Cannot apply configuration data when access mode is READ." },
  { GOGOC_UIS__CFG_CANNOTCANCELWHENREAD,
    "Cannot cancel configuration changes when access mode is READ."},
  { GOGOC_UIS__CFG_CANNOTOVERRIDESAMECONTENTS,
    "Cannot override configuration data with same contents." },
  { GOGOC_UIS__CFG_CANNOTOVERRIDEWHENREAD,
    "Cannot override configuration data when access mode is READ." },

  /* Then... GOGOCConfig strings */
  { GOGOC_UIS__GROUP_GOGOCCONFIG,    // title
    "gogoCLIENT Configuration Error" },
  { GOGOC_UIS__G6C_INVALIDCONF,
    "Invalid configuration." },
  { GOGOC_UIS__G6C_FAILLOADDFLTCONF,
    "Failed to load default configuration: No default configuration file"\
    " was provided during initialization." },
   { GOGOC_UIS__G6C_SUPPLYPASSWDWHENNOTANON,
   "Must supply password when authentication method is NOT anonymous." },
   { GOGOC_UIS__G6C_PROXYCINVALIDMODE,
    "Proxy client cannot be enabled with tunnel mode v6udpv4." },
  { GOGOC_UIS__G6C_KAINTERVALINVALID,
    "Keep-alive interval cannot be 0 when keep-alive is enabled." },
  { GOGOC_UIS__G6C_IFTUNV6V4ANDV6UDPV4REQUIRED,
    "(if_tunnel_v6v4=,if_tunnel_v6udpv4=)You must provide both V6V4 and "\
    "V6UDPV4 interfaces when using V6ANYV4 tunnel mode." },
  { GOGOC_UIS__G6C_IFTUNV6V4REQUIRED,
    "(if_tunnel_v6v4=)You must provide the V6V4 tunnel interface when using"\
    " V6V4 tunnel mode." },
  { GOGOC_UIS__G6C_IFTUNV6UDPV4REQUIRED,
    "(if_tunnel_v6udpv4=)You must provide the V6UDPV4 tunnel interface when"\
    " using V6UDPV4 tunnel mode." },
  { GOGOC_UIS__G6C_IFTUNV4V6REQUIRED,
    "(if_tunnel_v4v6=)You must provide the V4V6 tunnel interface when using"\
    " V4V6 tunnel mode." },

  /* The gogoc validation strings */
  { GOGOC_UIS__GROUP_GOGOCVALIDATION,    // title
    "gogoCLIENT Validation Error" },
  { GOGOC_UIS__G6V_USERIDTOOLONG,
    "(userid=)User ID must not be longer than 253 characters." },
  { GOGOC_UIS__G6V_USERIDINVALIDCHRS,
    "(userid=)Invalid characters found in user name." },
  { GOGOC_UIS__G6V_PASSWDTOOLONG,
    "(passwd=)Password must not be longer than 128 characters." },
  { GOGOC_UIS__G6V_PASSWDINVALIDCHRS,
    "(passwd=)Invalid characters found in password." },
  { GOGOC_UIS__G6V_SERVERMUSTBESPEC,
    "(server=)A server MUST be specified." },
  { GOGOC_UIS__G6V_SERVERTOOLONG,
    "(server=)Server must not be longer than 1025 characters." },
  { GOGOC_UIS__G6V_SERVERINVALIDCHRS,
    "(server=)Invalid characters found in server string." },
  { GOGOC_UIS__G6V_HOSTTYPEINVALIDVALUE,
    "(host_type=)Host type must be: <router|host>." },
  { GOGOC_UIS__G6V_PREFIXLENINVALIDVALUE,
    "(prefixlen=)Prefix length must be between 0 and 128." },
  { GOGOC_UIS__G6V_IFPREFIXINVALIDCHRS,
    "(if_prefix=)Invalid characters found in interface name." },
  { GOGOC_UIS__G6V_IFPREFIXMUSTBESPEC,
    "(if_prefix=)Interface prefix must be supplied when host_type is 'router'." },
  { GOGOC_UIS__G6V_DNSSERVERSTOOLONG,
    "(dns_server=)DNS servers string must not be longer than 1025 characters." },
  { GOGOC_UIS__G6V_DNSSERVERSUNRESOLVABLE,
    "(dns_server=)Failed to resolve one or more DNS servers found in configuration." },
  { GOGOC_UIS__G6V_GOGOCDIRDOESNTEXIST,
    "(gogoc_dir=)The directory does not exist." },
  { GOGOC_UIS__G6V_AUTHMETHODINVALIDVALUE,
    "(auth_method=)Authorization method must be: <anonymous|"\
                 "any|digest-md5|plain|passdss-3des-1>." },
  { GOGOC_UIS__G6V_AUTORETRYCONNECTINVALIDVALUE,
    "(retry_connect=)Retry connection must be:<yes|no>" },
  { GOGOC_UIS__G6V_RETRYDELAYINVALIDVALUE,
    "(retry_delay=)Retry delay must be between 0 and 3600." },
  { GOGOC_UIS__G6V_KEEPALIVEINVALIDVALUE,
    "(keepalive=)Keep-alive must be: <yes|no>" },
  { GOGOC_UIS__G6V_KEEPALIVEINTERVINVALID,
    "(keepalive_interval=)Keep-alive interval must be positive." },
  { GOGOC_UIS__G6V_TUNNELMODEINVALIDVALUE,
    "(tunnel_mode=)Tunnel mode must be: <v6anyv4|v6v4|v6udpv4|v4v6>" },
  { GOGOC_UIS__G6V_IFTUNV6V4INVALIDCHRS,
    "(if_tunnel_v6v4=)Invalid characters found in interface name." },
  { GOGOC_UIS__G6V_IFTUNV6UDPV4INVALIDCHRS,
    "(if_tunnel_v6udpv4=)Invalid characters found in interface name." },
  { GOGOC_UIS__G6V_IFTUNV4V6INVALIDCHRS,
    "(if_tunnel_v4v6=)Invalid characters found in interface name." },
  { GOGOC_UIS__G6V_CLIENTV4INVALIDVALUE,
    "(client_v4=)Invalid IPv4 address. Address must be: <auto|A.B.C.D>." },
  { GOGOC_UIS__G6V_CLIENTV6INVALIDVALUE,
    "(client_v6=)IPv6 address must be: <auto|X:X::X:X>." },
  { GOGOC_UIS__G6V_TEMPLATEINVALIDVALUE,
    "(template=)Template must be: <checktunnel|freebsd|netbsd|"\
                 "linux|windows|darwin|cisco|solaris|openbsd|openwrt|gogocpe>" },
  { GOGOC_UIS__G6V_PROXYCLIENTINVALIDVALUE,
    "(proxy_client=)Proxy client must be: <yes|no>" },
  { GOGOC_UIS__G6V_BROKERLISTTOOLONG,
    "(broker_list=)Broker list filename cannot be greater than 256 characters." },
  { GOGOC_UIS__G6V_BROKERLISTINVALIDCHRS,
    "(broker_list=)Invalid characters found in broker list file name." },
  { GOGOC_UIS__G6V_LASTSERVTOOLONG,
    "(last_server=)Last server filename cannot be greater than 256 characters." },
  { GOGOC_UIS__G6V_LASTSERVINVALIDCHRS,
    "(last_server=)Invalid characters found in last server file name." },
  { GOGOC_UIS__G6V_ALWAYSUSERLASTSERVINVALIDVALUE,
    "(always_use_same_server=)Value must be: <yes|no>" },
  { GOGOC_UIS__G6V_LOGLEVELINVALIDVALUE,
    "(log=)Log level must be between 0 and 3." },
  { GOGOC_UIS__G6V_LOGDEVICEINVALIDVALUE,
    "(log=)Log device must be: <console|stderr|file|syslog>" },
  { GOGOC_UIS__G6V_LOGFILENAMETOOLONG,
    "(log_filename=)Log filename cannot be greater than 256 characters." },
  { GOGOC_UIS__G6V_LOGFILENAMEINVALIDCHRS,
    "(log_filename=)Invalid characters found in log file name." },
  { GOGOC_UIS__G6V_LOGROTATIONINVALIDVALUE,
    "(log_rotation=)Log rotation must be: <yes|no>" },
  { GOGOC_UIS__G6V_LOGROTSZINVALIDVALUE,
    "(log_rotation_size=)Log rotation size(in KB) must be: <16|32|128|1024>" },
  { GOGOC_UIS__G6V_LOGROTDELINVALIDVALUE,
    "(log_rotation_delete=)Log rotation deletion must be: <yes|no>" },
  { GOGOC_UIS__G6V_SYSLOGFACILITYINVALIDVALUE,
    "(syslog_facility=)Syslog facility must be: <USER|LOCAL0|"\
                 "LOCAL1|LOCAL2|LOCAL3|LOCAL4|LOCAL5|LOCAL6|LOCAL7>" },
  { GOGOC_UIS__G6V_DNSSERVERSINVALIDCHRS,
    "(dns_server=)One or more DNS server specified contains invalid characters." },
  { GOGOC_UIS__G6V_HACCESSPROXYENABLEDINVALIDVALUE,
    "(haccess_proxy_enabled=)Home Access must be: <yes|no>" },
  { GOGOC_UIS__G6V_HACCESSWEBENABLEDINVALIDVALUE,
    "(haccess_web_enabled=)Home Web must be: <yes|no>" },
  { GOGOC_UIS__G6V_HACCESSDOCROOTDOESNTEXIST,
    "(haccess_document_root=)Home Web document root must exist." },
  { GOGOC_UIS__G6V_HACCESSDOCROOTNOTSPEC,
    "(haccess_document_root=)Must be specified when haccess_web_enabled=yes." },
  { GOGOC_UIS__G6V_HACCESSINCOMPV4V6,
    "(tunnel_mode=)Tunnel mode cannot be V4V6 with Home Access or Home Web." },
  { GOGOC_UIS__G6C_PROXYANDKEEPALIVE,
    "(keepalive=)Keep-alives cannot be turned on when proxy mode is on." },
  { GOGOC_UIS__G6V_RETRYDELAYMAXINVALIDVALUE,
    "(retry_delay_max=)Retry delay max must be between 0 and 3600." },
  { GOGOC_UIS__G6V_RETRYDELAYGREATERRETRYDELAYMAX,
    "(retry_delay_max=)Retry delay max must be greater than retry delay." }
};


// --------------------------------------------------------------------------
// Function : get_ui_string
//
// Description:
//   Returns the user interface string specified by the id.
//
// Arguments:
//   id: int [IN], The string ID.
//
// Return values:
//   The UI string.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
const char* get_ui_string( const error_t id )
{
  const unsigned int n = sizeof(gogocUIStrings) / sizeof(gogocUIStrings[0]);
  unsigned int i;

  for(i=0; i<n; i++)
    if(gogocUIStrings[i]._id == id)
      return gogocUIStrings[i]._str;

  return (const char*)0;    // NULL
}
