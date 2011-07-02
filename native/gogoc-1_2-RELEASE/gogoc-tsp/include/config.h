/*
-----------------------------------------------------------------------------
 $Id: config.h,v 1.3 2010/03/07 20:05:46 carl Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/
#ifndef CONFIG_H
#define CONFIG_H


/* these globals are defined by US used by alot of things in  */
#define STR_CONFIG_TUNNELMODE_V6ANYV4   "v6anyv4"
#define STR_CONFIG_TUNNELMODE_V6V4      "v6v4"
#define STR_CONFIG_TUNNELMODE_V6UDPV4   "v6udpv4"
#define STR_CONFIG_TUNNELMODE_V4V6      "v4v6"
#define STR_CONFIG_TUNNELMODE_DSLITE    "dslite"

#define STR_XML_TUNNELMODE_V6ANYV4    "v6anyv4"
#define STR_XML_TUNNELMODE_V6V4       "v6v4"
#define STR_XML_TUNNELMODE_V6UDPV4    "v6udpv4"
#define STR_XML_TUNNELMODE_V4V6       "v4v6"

#ifdef FALSE
#undef FALSE
#endif

#ifdef TRUE
#undef TRUE
#endif

#define STR_CONFIG_BOOLEAN_FALSE  "no"
#define STR_CONFIG_BOOLEAN_TRUE   "yes"


typedef enum
{
  FALSE=0,
  TRUE
} tBoolean;

typedef enum
{
  V6V4=1,
  V6UDPV4=2,
  V6ANYV4=3,
  V4V6=4,
  DSLITE=5,
} tTunnelMode;


typedef struct stConf {
  char *tsp_dir,
       *server,
       *dslite_server,
       *dslite_client,
       *userid,
       *passwd,
       *auth_method,
       *client_v4,
       *client_v6,
       *protocol,
       *if_tunnel_v6v4,
       *if_tunnel_v6udpv4,
       *if_tunnel_v4v6,
       *dns_server,
       *routing_info,
       *if_prefix,
       *template,
       *host_type,
       *log_filename,
       *last_server_file,
       *haccess_document_root,
       *broker_list_file;
  sint32_t keepalive_interval;
  sint32_t prefixlen;
  sint32_t retry_delay;
  sint32_t retry_delay_max;
  sint32_t syslog_facility;
  sint32_t transport;
  sint32_t log_rotation_size;
  sint16_t log_level_stderr;
  sint16_t log_level_syslog;
  sint16_t log_level_console;
  sint16_t log_level_file;
  tBoolean keepalive;
  tBoolean syslog;
  tBoolean proxy_client;
  tBoolean log_rotation;
  tBoolean log_rotation_delete;
  tBoolean always_use_same_server;
  tBoolean auto_retry_connect;
  tTunnelMode tunnel_mode;
  tBoolean haccess_web_enabled;
  tBoolean haccess_proxy_enabled;
  tBoolean boot_mode;
  tBoolean nodaemon;
  tBoolean no_questions;

  // These are run-time, dynamically computed values
  //
  char addr_local_v4[INET6_ADDRSTRLEN];
  uint16_t port_local_v4;
  char addr_remote_v4[INET6_ADDRSTRLEN];
  uint16_t port_remote_v4;
} tConf;


typedef struct syslog_facility
{
  char *string;
  sint32_t value;
} syslog_facility_t;


/* Valid syslog_facility values */
#define STR_CONFIG_SLOG_FACILITY_USER   "USER"
#define STR_CONFIG_SLOG_FACILITY_LOCAL0 "LOCAL0"
#define STR_CONFIG_SLOG_FACILITY_LOCAL1 "LOCAL1"
#define STR_CONFIG_SLOG_FACILITY_LOCAL2 "LOCAL2"
#define STR_CONFIG_SLOG_FACILITY_LOCAL3 "LOCAL3"
#define STR_CONFIG_SLOG_FACILITY_LOCAL4 "LOCAL4"
#define STR_CONFIG_SLOG_FACILITY_LOCAL5 "LOCAL5"
#define STR_CONFIG_SLOG_FACILITY_LOCAL6 "LOCAL6"
#define STR_CONFIG_SLOG_FACILITY_LOCAL7 "LOCAL7"


/* Valid log values */
#define STR_CONFIG_LOG_DESTINATION_STDERR   "stderr"
#define STR_CONFIG_LOG_DESTINATION_SYSLOG   "syslog"
#define STR_CONFIG_LOG_DESTINATION_CONSOLE  "console"
#define STR_CONFIG_LOG_DESTINATION_FILE     "file"


/* imports defined in the platform dependant file */
extern char *FileName;
extern char *LogFile;
extern char *ScriptInterpretor;
extern char *ScriptExtension;
extern char *ScriptDir;
extern char *TspHomeDir;
extern char DirSeparator;


/* functions exported */
gogoc_status         tspInitialize         (sint32_t, char *[], tConf *);

#endif
