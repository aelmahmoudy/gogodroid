/*
-----------------------------------------------------------------------------
 $Id: cli.c,v 1.2 2010/03/07 20:12:49 carl Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"
#include "gogoc_status.h"

#include "config.h"
#include "cli.h"
#include "log.h"  // Verbose
#include "hex_strings.h"
#include "lib.h"

#if defined(WIN32) && !defined(WINCE)
#include "console.h"
#include "service.h"
#endif


// --------------------------------------------------------------------------
void PrintUsage( char *message )
{
  // Print the message, if a message was passed.
  if( message )
  {
    printf(message);
  }

  // Print the usage.
  printf("usage: gogoc [options] [-f config_file] [-r seconds]\n"
          "  where options are :\n"
          "    -i    gif interface to use for tunnel_v6v4\n"
          "    -u    interface to use for tunnel_v6udpv4\n"
          "    -s    interface to query to get IPv4 source address\n"
          "    -f    Read this config file instead of %s \n"
          "    -r    Retry after n seconds until success\n"
          "    -b    Boot mode: avoid reconnecting after failure\n"
          "    -n    Run in foreground\n"
          "    -y    Do not ask y/n questions\n"
#if defined(WIN32) && !defined(WINCE)
          "    --register    install to run as service\n"
          "    --unregister  uninstall the service\n"
#endif
          "    -h    help\n"
          "    -?    help\n\n", FileName);
}


// --------------------------------------------------------------------------
void ParseArguments(sint32_t argc, char *argv[], tConf *Conf)
{
  sint32_t ch;


#if defined(WIN32) && !defined(WINCE)
  // Platform HACK!
  service_parse_cli(argc, argv);
#endif

  while( (ch = pal_getopt(argc, argv, "h?b?n?y?f:r:i:u:s:")) != -1 )
  {
    switch( ch )
    {
      case 'b':
        Conf->boot_mode = TRUE;
        break;
      case 'n':
        Conf->nodaemon = TRUE;
        break;
      case 'y':
        Conf->no_questions = TRUE;
        break;
      case 's':
        Conf->client_v4 = optarg;
        break;
      case 'i':
        Conf->if_tunnel_v6v4 = optarg;
        break;
      case 'u':
        Conf->if_tunnel_v6udpv4 = optarg;
        break;
      case 'f':
        FileName = optarg;
        break;
      case 'r':
        Conf->retry_delay = atoi(optarg);
        break;
      case '?':
      case 'h':
        PrintUsage(NULL);
        exit(0);

      default:
        PrintUsage("Error while parsing command line arguments");
        exit(1);
    }
  }
}


// --------------------------------------------------------------------------
// Ask the question, return 0 if answer is N or n, 1 if answer is Y or y.
// WINCE does not have console
// This function is a travesty in a daemon or service
//
#if !defined(WINCE)
sint32_t ask(char *question, ...)
{
  va_list ap;
  char *buf;
  sint32_t c;
  sint32_t ret;

#ifdef WIN32
  enable_console_input();
#endif

  if ( (buf = malloc(sizeof(char) * 1024)) == NULL ) {
    Display(LOG_LEVEL_1, ELError, "ask", STR_GEN_MALLOC_ERROR);
    return 0;
  }

  va_start(ap, question);
  pal_vsnprintf(buf, 1024, question, ap);
  va_end(ap);

ask_again:

  printf("%s? (Y/N) ", buf);

  c = fgetc(stdin);

  /* empty stdin */
  fflush(stdin);

  c = tolower(c);

  if ((char)c == 'y')
    ret = 1;
  else if ((char)c == 'n')
    ret = 0;
  else goto ask_again;

  free(buf);

#ifdef WIN32
  disable_console_input();
#endif

  return ret;
}
#endif
