/*
-----------------------------------------------------------------------------
 $Id: config.c,v 1.3 2010/03/07 20:12:49 carl Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

/*  Configuration file handling. */

#include "platform.h"
#include "gogoc_status.h"

#include "config.h"
#include "log.h"
#include "hex_strings.h"
#include "cli.h"

/* gogoCLIENT Configuration Subsystem */
#define TBOOLEAN_DECLARED
#include <gogocconfig/gogoc_c_wrapper.h>
#include <gogocconfig/gogocuistrings.h>
#undef TBOOLEAN_DECLARED


#if !(defined(WIN32) || defined(WINCE))
static syslog_facility_t syslog_facilities[] = {
  { STR_CONFIG_SLOG_FACILITY_USER, LOG_USER },
  { STR_CONFIG_SLOG_FACILITY_LOCAL0, LOG_LOCAL0 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL1, LOG_LOCAL1 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL2, LOG_LOCAL2 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL3, LOG_LOCAL3 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL4, LOG_LOCAL4 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL5, LOG_LOCAL5 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL6, LOG_LOCAL6 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL7, LOG_LOCAL7 },
  { NULL, 0 }
};


/* ----------------------------------------------------------------------- */
/* Function: ParseSyslogFacility                                           */
/*                                                                         */
/* Description:                                                            */
/*   Parse the configuration file's 'syslog_facility' directive.           */
/*                                                                         */
/* Arguments:                                                              */
/*   pConf: tConf* [OUT], The global configuration object.                 */
/*   facility: char* [IN], The input syslog facility.                      */
/*                                                                         */
/* Return Values:                                                          */
/*   1 on error.                                                           */
/*   0 on successful completion.                                           */
/*                                                                         */
/* ----------------------------------------------------------------------- */
static sint32_t ParseSyslogFacility( tConf *pConf, char *facility )
{
  sint32_t index = 0;

  /* Loop through the known facility strings, and compare with the one we found. */
  while( (syslog_facilities != NULL) && (syslog_facilities[index].string != NULL) )
  {
    if (strcmp(facility, syslog_facilities[index].string) == 0)
    {
      pConf->syslog_facility = syslog_facilities[index].value;
      return 0;
    }
    index++;
  }

  return 1;
}
#endif


/* ----------------------------------------------------------------------- */
/* Function: tspReadConfigFile                                             */
/*                                                                         */
/* Description:                                                            */
/*   Will extract the configuration data from the configuration file.      */
/*                                                                         */
/* Arguments:                                                              */
/*   szFile: char* [IN], The input configuration filename.                 */
/*   pConf: tConf* [OUT], The global configuration object.                 */
/*                                                                         */
/* Return Values:                                                          */
/*   gogoc_err value.                                                       */
/*                                                                         */
/* ----------------------------------------------------------------------- */
#ifdef ANDROID
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define LINESIZE 256
static ssize_t readline(char **lineptr, size_t * len, FILE *input)
{
  char *ptr;
  size_t size, count = 0;

  if (input == NULL) {
    return -1;
  }

  if (*lineptr == NULL) {
    if ((*lineptr = malloc(LINESIZE)) == NULL) {
      return -1;
    }
    *len = LINESIZE;
  }

  for (ptr = *lineptr; (size = fread(ptr, 1, 1, input)) > 0; ptr++) {
    if (++count == *len) {
      if (realloc(*lineptr, *len + LINESIZE) == NULL) {
        return -1;
      }
      *len += LINESIZE;
    }

    if (*ptr == '\n') {
      *ptr = '\0';
      return count;
    }
  }

  if (size == 0) {
    *ptr = '\0';
    return count;
  }

  return -1;
}

gogoc_status tspReadConfigFile( char* szFile, tConf* pConf )
{
  size_t len = 0;
  ssize_t count;
  char *line = NULL;
  char *name = NULL, *value = NULL;
  FILE *input;

  pConf->auth_method = pal_strdup("anonymous");
  pConf->server = pal_strdup("anonymous.freenet6.net");
  pConf->tsp_dir = pal_strdup("/data/data/org.nklog.gogoc");
  pConf->template = pal_strdup("android");
  pConf->broker_list_file = pal_strdup("/data/data/org.nklog.gogoc/files/broker_list_file");
  pConf->last_server_file = pal_strdup("/data/data/org.nklog.gogoc/files/last_server_file");

  pConf->if_prefix = pal_strdup("");
  pConf->dns_server = pal_strdup("");
  pConf->tunnel_mode = V6ANYV4;
  pConf->if_tunnel_v6v4 = pal_strdup("sit1");
  pConf->if_tunnel_v6udpv4 = pal_strdup("tun");
  pConf->if_tunnel_v4v6 = pal_strdup("sit0");
  pConf->userid = pal_strdup("");
  pConf->passwd = pal_strdup("");

  pConf->host_type = pal_strdup("host");
  pConf->prefixlen = 64;

  pConf->auto_retry_connect = TRUE;
  pConf->retry_delay = 30;
  pConf->retry_delay_max = 300;
  pConf->keepalive = TRUE;
  pConf->keepalive_interval = 30;

  pConf->client_v4 = pal_strdup("auto");
  pConf->client_v6 = pal_strdup("auto");
  pConf->proxy_client = FALSE;
  pConf->always_use_same_server = FALSE;

  pConf->log_level_console = 0;

  pConf->log_level_stderr = 0;

  pConf->log_level_syslog = 0;
  ParseSyslogFacility(pConf, "USER");

  pConf->log_level_file = 3;
  pConf->log_filename = pal_strdup("/data/data/org.nklog.gogoc/files/gogoc.log");
  pConf->log_rotation = TRUE;
  pConf->log_rotation_size = 32;
  pConf->log_rotation_delete = TRUE;

  input = fopen(szFile, "r");
  while ((count = readline(&line, &len, input)) > 0) {
    if (*line == '#' || *line == '\0') {
      continue;
    }
    name = line;
    if ((value = strchr(line, '=')) != NULL) {
      *(value++) = '\0';
    }
    if (strcmp(name, "userid") == 0) {
      pConf->userid = pal_strdup(value);
    } else if (strcmp(name, "passwd") == 0) {
      pConf->passwd = pal_strdup(value);
    } else if (strcmp(name, "server") == 0) {
      pConf->server = pal_strdup(value);
    } else if (strcmp(name, "auth_method") == 0) {
      pConf->auth_method = pal_strdup(value);
    } else if (strcmp(name, "gogoc_dir") == 0) {
      pConf->tsp_dir = pal_strdup(value);
    } else if (strcmp(name, "template") == 0) {
      pConf->template = pal_strdup(value);
    } else if (strcmp(name, "broker_list") == 0) {
      pConf->broker_list_file = pal_strdup(value);
    } else if (strcmp(name, "last_server") == 0) {
      pConf->last_server_file = pal_strdup(value);
    } else if (strcmp(name, "if_prefix") == 0) {
      pConf->if_prefix = pal_strdup(value);
    } else if (strcmp(name, "dns_server") == 0) {
      pConf->dns_server = pal_strdup(value);
    } else if (strcmp(name, "tunnel_mode") == 0) {
      if (strcmp(value, "v6v4") == 0) {
        pConf->tunnel_mode = V6V4;
      } else if (strcmp(value, "v6udpv4") == 0) {
        pConf->tunnel_mode = V6UDPV4;
      } else if (strcmp(value, "v4v6") == 0) {
        pConf->tunnel_mode = V4V6;
      } else if (strcmp(value, "v6udpv4") == 0) {
        pConf->tunnel_mode = V6UDPV4;
      } else if (strcmp(value, "v6anyv4") == 0) {
        pConf->tunnel_mode = V6ANYV4;
      }
    } else if (strcmp(name, "if_tunnel_v6v4") == 0) {
      pConf->if_tunnel_v6v4 = pal_strdup(value);
    } else if (strcmp(name, "if_tunnel_v6udpv4") == 0) {
      pConf->if_tunnel_v6udpv4 = pal_strdup(value);
    } else if (strcmp(name, "if_tunnel_v4v6") == 0) {
      pConf->if_tunnel_v4v6 = pal_strdup(value);
    }
  }
  if (input != NULL) {
    fclose(input);
  }

  return make_status(CTX_CFGVALIDATION, SUCCESS);
}
#else
gogoc_status tspReadConfigFile( char* szFile, tConf* pConf )
{
  sint32_t i, nErrors, iRet;
  uint32_t* tErrors = NULL;
  char* szValue = NULL;


  /* Check input parameters. */
  if( szFile == NULL  ||  pConf == NULL )
  {
    return make_status(CTX_CFGVALIDATION, ERR_INVAL_CFG_FILE);
  }


  /* --------------------------------------------------------------------- */
  /* Read and load the configuration file.                                 */
  /* Will also perform thorough validation.                                */
  /* --------------------------------------------------------------------- */
  if( (iRet = initialize( szFile )) != 0 )
  {
    if( iRet == -1 )
    {
      /* Retrieve confguration error(s). */
      get_config_errors( &nErrors, &tErrors );

      for( i=0; i<nErrors; i++ )
        DirectErrorMessage( (char*)get_ui_string( tErrors[i] ) );
    }
    else
    {
      /* Initialization error */
      DirectErrorMessage( (char*)get_ui_string( iRet ) );
    }

    return make_status(CTX_CFGVALIDATION, ERR_INVAL_CFG_FILE);
  }


  /* --------------------------------------------------------------------- */
  /* Fill in the tConf structure from the file contents.                   */
  /* --------------------------------------------------------------------- */

  // Server is facultative in the gogoc-config validation routine, but not here.
  get_server( &(pConf->server) );
  if( strlen( pConf->server ) == 0 )
  {
    free( pConf->server );
    DirectErrorMessage( (char*)get_ui_string( GOGOC_UIS__G6V_SERVERMUSTBESPEC ) );
    return make_status(CTX_CFGVALIDATION, ERR_INVAL_CFG_FILE);
  }

  get_gogoc_dir( &(pConf->tsp_dir) );

#ifdef DSLITE_SUPPORT
  get_dslite_server( &(pConf->dslite_server) );
  get_dslite_client( &(pConf->dslite_client) );
#endif
  
  get_client_v4( &(pConf->client_v4) );

#ifdef V4V6_SUPPORT
  get_client_v6( &(pConf->client_v6) );
#endif /* V4V6_SUPPORT  */

  get_user_id( &(pConf->userid) );

  get_passwd( &(pConf->passwd) );

  get_auth_method( &(pConf->auth_method) );

  get_host_type( &(pConf->host_type) );

  get_template( &(pConf->template) );

  get_if_tun_v6v4( &(pConf->if_tunnel_v6v4) );

  get_if_tun_v6udpv4( &(pConf->if_tunnel_v6udpv4) );

#ifdef V4V6_SUPPORT
  get_if_tun_v4v6( &(pConf->if_tunnel_v4v6) );
#endif /* V4V6_SUPPORT */

  get_tunnel_mode( &szValue );

  if (strcmp(szValue, STR_CONFIG_TUNNELMODE_V6ANYV4) == 0) {
    pConf->tunnel_mode = V6ANYV4;
  }
  else if (strcmp(szValue, STR_CONFIG_TUNNELMODE_V6V4) == 0) {
    pConf->tunnel_mode = V6V4;
  }
  else if (strcmp(szValue, STR_CONFIG_TUNNELMODE_V6UDPV4) == 0) {
    pConf->tunnel_mode = V6UDPV4;
  }
#ifdef V4V6_SUPPORT
  else if (strcmp(szValue, STR_CONFIG_TUNNELMODE_V4V6) == 0) {
    pConf->tunnel_mode = V4V6;
  }
#endif /* V4V6_SUPPORT */
#ifdef DSLITE_SUPPORT
  else if (strcmp(szValue, STR_CONFIG_TUNNELMODE_DSLITE) == 0) {
    pConf->tunnel_mode = DSLITE;
  }
#endif /* V4V6_SUPPORT */
  free( szValue );  szValue = NULL;

  get_dns_server( &(pConf->dns_server) );

  get_ifprefix( &(pConf->if_prefix) );

  get_prefixlen( &(pConf->prefixlen) );

  get_retry_delay( &(pConf->retry_delay) );

  get_retry_delay_max( &(pConf->retry_delay_max) );

  get_keepalive( &(pConf->keepalive) );

  get_keepalive_interval( &(pConf->keepalive_interval) );

  get_proxy_client( &(pConf->proxy_client) );

#if !(defined(WIN32) || defined(WINCE))
  get_syslog_facility( &szValue );
  ParseSyslogFacility( pConf, szValue );
  free( szValue );  szValue = NULL;
#endif

  get_log_filename( &(pConf->log_filename) );

  get_log_rotation( &(pConf->log_rotation) );

  get_log_rotation_sz( &(pConf->log_rotation_size) );

  get_log_rotation_del( &(pConf->log_rotation_delete) );

  get_log( STR_CONFIG_LOG_DESTINATION_STDERR, &(pConf->log_level_stderr) );

  get_log( STR_CONFIG_LOG_DESTINATION_SYSLOG, &(pConf->log_level_syslog) );

  get_log( STR_CONFIG_LOG_DESTINATION_CONSOLE, &(pConf->log_level_console) );

  get_log( STR_CONFIG_LOG_DESTINATION_FILE, &(pConf->log_level_file) );

  get_auto_retry_connect( &(pConf->auto_retry_connect) );

  get_last_server_file( &(pConf->last_server_file) );

  get_always_use_last_server( &(pConf->always_use_same_server) );

  get_broker_list_file( &(pConf->broker_list_file) );

  get_haccess_web_enabled( &(pConf->haccess_web_enabled) );

  get_haccess_proxy_enabled( &(pConf->haccess_proxy_enabled) );

  get_haccess_document_root( &(pConf->haccess_document_root) );

  /* Close the gogoCLIENT configuration object. */
  un_initialize();

  /* Successful completion. */
  return make_status(CTX_CFGVALIDATION, SUCCESS);
}
#endif


/* ----------------------------------------------------------------------- */
/* Function: tspInitialize                                                 */
/*                                                                         */
/* Description:                                                            */
/*   Initialize with default values, read configuration file and override  */
/*   defaults with config file values.                                     */
/*                                                                         */
/* Arguments:                                                              */
/*   argc: char* [IN], Number of arguments passed on command line.         */
/*   argv: char*[] [IN], The command-line arguments.                       */
/*   pConf: tConf* [OUT], The global configuration object.                 */
/*                                                                         */
/* Return Values:                                                          */
/*   gogoc_err value                                                        */
/*                                                                         */
/* ----------------------------------------------------------------------- */
gogoc_status tspInitialize(sint32_t argc, char *argv[], tConf *pConf)
{
  tConf CmdLine;
  gogoc_status status = STATUS_SUCCESS_INIT;
  const char* cszTemplDir = "template";


  // Hard-coded parameters. Not configurable anymore.
  pConf->syslog = FALSE;
  pConf->protocol = pal_strdup( "default_route" );
  pConf->routing_info = pal_strdup("");


  /* --------------------------------------------------------------------- */
  /* Read configuration data from the command-line arguments.              */
  /* --------------------------------------------------------------------- */
  memset(&CmdLine, 0, sizeof(CmdLine));
  if( argc > 1 )
  {
    ParseArguments(argc, argv, &CmdLine);
  }

  /* --------------------------------------------------------------------- */
  /* Read configuration data from the file.                                */
  /* --------------------------------------------------------------------- */
  status = tspReadConfigFile(FileName, pConf);
  if( status_number(status) != SUCCESS )
  {
    return status;
  }

  /* --------------------------------------------------------------------- */
  /* Override the config file with parameters from the command line.       */
  /* --------------------------------------------------------------------- */
  if(CmdLine.if_tunnel_v6v4)
    pConf->if_tunnel_v6v4 = CmdLine.if_tunnel_v6v4;

  if(CmdLine.if_tunnel_v6udpv4)
    pConf->if_tunnel_v6udpv4 = CmdLine.if_tunnel_v6udpv4;

#ifdef V4V6_SUPPORT
  if(CmdLine.if_tunnel_v4v6)
    pConf->if_tunnel_v4v6 = CmdLine.if_tunnel_v4v6;
#endif /* V4V6_SUPPORT  */

  if(CmdLine.client_v4)
    pConf->client_v4 = CmdLine.client_v4;

  pConf->boot_mode = CmdLine.boot_mode;
  pConf->nodaemon = CmdLine.nodaemon;
  pConf->no_questions = CmdLine.no_questions;

  /* --------------------------------------------------------------------- */
  /* Extrapolate directory in which template scripts are located.          */
  /* --------------------------------------------------------------------- */
  if( strlen(pConf->tsp_dir) != 0 )
  {
    TspHomeDir = pConf->tsp_dir;
    if( (ScriptDir = (char*)malloc( (size_t)(strlen(pConf->tsp_dir)+strlen(cszTemplDir)+2)) ) == NULL )
    {
      DirectErrorMessage( STR_GEN_MALLOC_ERROR );
      return make_status(CTX_CFGVALIDATION, ERR_MEMORY_STARVATION);
    }
    sprintf(ScriptDir, "%s%c%s", pConf->tsp_dir, DirSeparator, cszTemplDir);
  }
  else
  {
    if((ScriptDir = (char *)malloc((size_t)(strlen(TspHomeDir)+strlen(cszTemplDir)+2)))==NULL)
    {
      DirectErrorMessage( STR_GEN_MALLOC_ERROR );
      return make_status(CTX_CFGVALIDATION, ERR_MEMORY_STARVATION);
    }
    sprintf(ScriptDir, "%s%c%s", TspHomeDir, DirSeparator, cszTemplDir);
  }

  return make_status(CTX_CFGVALIDATION, SUCCESS);
}
