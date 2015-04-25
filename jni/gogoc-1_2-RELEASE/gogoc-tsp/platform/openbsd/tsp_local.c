/*
-----------------------------------------------------------------------------
 $Id: tsp_local.c,v 1.3 2010/03/07 20:09:28 carl Exp $
-----------------------------------------------------------------------------
Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
  
-----------------------------------------------------------------------------
*/

/* OPENBSD */


#include "platform.h"
#include "gogoc_status.h"

#include <sys/wait.h>

#include "config.h" /* tConf */
#include "xml_tun.h" /* tTunnel */
#include "net.h"  /* net_tools_t */
#include "tsp_setup.h"
#include "tsp_net.h"
#include "tsp_tun_mgt.h"  // tspPerformTunnelLoop()

#include "log.h"
#include "hex_strings.h"

#include "tsp_tun.h"

#define IFNAMSIZ 16     // from net/if.h

/* these globals are defined by US used by alot of things in  */

char *FileName  = "gogoc.conf";
char *ScriptInterpretor = "/bin/sh";
char *ScriptExtension = "sh";
char *ScriptDir = NULL;
char *TspHomeDir = "/usr/local/etc/gogoc";
char DirSeparator = '/';

int indSigHUP = 0;    // Set to 1 when HUP signal is trapped.


#include <gogocmessaging/gogocuistrings.h>
// Dummy implementation for non-win32 targets
// (Library gogocmessaging is not linked in non-win32 targets).
error_t send_status_info( void ) { return GOGOCM_UIS__NOERROR; }
error_t send_tunnel_info( void ) { return GOGOCM_UIS__NOERROR; }
error_t send_broker_list( void ) { return GOGOCM_UIS__NOERROR; }
error_t send_haccess_status_info( void ) { return GOGOCM_UIS__NOERROR; }


// --------------------------------------------------------------------------
// Platform specific way to set an environment variable.
//
void tspSetEnv(char *Variable, char *Value, int Flag)
{
  Display(LOG_LEVEL_3, ELInfo, "tspSetEnv", GOGO_STR_ENV_PRINT_VALUE, Variable, Value);
  setenv(Variable, Value, Flag);
}

// --------------------------------------------------------------------------
// Checks if the gogoCLIENT has been requested to stop and exit.
//
// Returns 1 if gogoCLIENT is being requested to stop and exit.
// Else, waits 'uiWaitMs' miliseconds and returns 0.
//
// Defined in tsp_client.h
//
int tspCheckForStopOrWait( const unsigned int uiWaitMs )
{
  // Sleep for the amount of time specified, if signal has not been sent.
  if( indSigHUP == 0 )
  {
    // usleep is expecting microseconds (1 microsecond = 0.000001 second).
    usleep( uiWaitMs * 1000 );
  }

  return indSigHUP;
}


// --------------------------------------------------------------------------
// Called from tsp_setup.c -> tspSetupInterface
//   Do extra platform-specific stuff before tunnel script is launched.
//
int tspSetupInterfaceLocal( tConf* pConf, tTunnel* pTun )
{
  return 0;
}


// --------------------------------------------------------------------------
/* tspSetupTunnel() will callback here */
//
char* tspGetLocalAddress(int socket, char *buffer, int size)
{
  struct sockaddr_in6 addr; /* enough place for v4 and v6 */
  struct sockaddr_in  *addr_v4 = (struct sockaddr_in *)&addr;
  struct sockaddr_in6 *addr_v6 = (struct sockaddr_in6 *)&addr;
  int len;

  len = sizeof addr;
  if (getsockname(socket, (struct sockaddr *)&addr, &len) < 0) {
    Display(LOG_LEVEL_1, ELError, "TryServer", GOGO_STR_ERR_FIND_SRC_IP);
    return NULL;
  }

  if (addr.sin6_family == AF_INET6)
    return (char *)inet_ntop(AF_INET6, (const void*) &addr_v6->sin6_addr, buffer, size);
  else
    return (char *)inet_ntop(AF_INET, (const void*) &addr_v4->sin_addr, buffer, size);
}

// --------------------------------------------------------------------------
/* tspSetupTunnel() from tsp_client.c will callback here */
/* start locally, ie, setup interface and any daemons or anything needed */
//
#ifdef DSLITE_SUPPORT
#error DSLITE support not implemented on platform
#endif
gogoc_status tspStartLocal(int socket, tConf *c, tTunnel *t, net_tools_t *nt)
{
  TUNNEL_LOOP_CONFIG tun_loop_cfg;
  gogoc_status status = STATUS_SUCCESS_INIT;
  int ka_interval = 0;
  int tunfd = -1;
  int pid;


  /* Test for root privileges */
  if(geteuid() != 0)
  {
    Display(LOG_LEVEL_1, ELError, "tspStartLocal", GOGO_STR_FATAL_NOT_ROOT_FOR_TUN);
    return make_status(CTX_TUNINTERFACESETUP, ERR_INTERFACE_SETUP_FAILED);
  }

  // Check if we're already daemon. Calling multiple times the daemon() messes up pthreads.
  if( !c->nodaemon && getppid() != 1 )
  {
    Display(LOG_LEVEL_1, ELInfo, "tspStartLocal", GOGO_STR_GOING_DAEMON);
    if (daemon(1, 0) == -1)
    {
      Display(LOG_LEVEL_1, ELError, "tspStartLocal", GOGO_STR_CANT_FORK);
      return make_status(CTX_TUNINTERFACESETUP, ERR_INTERFACE_SETUP_FAILED);
    }
  }

  if (strcasecmp(t->type, STR_CONFIG_TUNNELMODE_V6UDPV4) == 0 )
  {
    // When using V6UDPV4 encapsulation, open the TUN device.
		tunfd = TunInit(c->if_tunnel_v6udpv4);
		if( tunfd == -1 )
		{
		  // Error: Failed to open TUN device.
		  Display(LOG_LEVEL_1, ELError, "tspStartLocal", STR_MISC_FAIL_TUN_INIT);
		  return make_status(CTX_TUNINTERFACESETUP, ERR_INTERFACE_SETUP_FAILED);
		}

    // Get the real name of the opened tun device for the template script.
    free( c->if_tunnel_v6udpv4 );
    c->if_tunnel_v6udpv4 = (char*) malloc( IFNAMSIZ );
    TunName(tunfd, c->if_tunnel_v6udpv4, IFNAMSIZ );
  }
  else if (strcasecmp(t->type, STR_CONFIG_TUNNELMODE_V4V6) == 0 )
  {
    Display(LOG_LEVEL_1, ELError, "tspStartLocal", GOGO_STR_NO_V4V6_ON_PLATFORM);
    return(make_status(CTX_TUNINTERFACESETUP, ERR_INTERFACE_SETUP_FAILED));
  }


  while( 1 ) // Dummy loop. 'break' instruction at the end.
  {
    // Run the config script in another thread, without giving it our tunnel
    // descriptor. This is important because otherwise the tunnel will stay
    // open if we get killed.
    //
    pid = fork();

    if( pid < 0 )
    {
      // fork() error
      status = make_status(CTX_TUNINTERFACESETUP, ERR_INTERFACE_SETUP_FAILED);
      break;
    }
    else if( pid == 0 )
    {
      // Child processing: run template script.
      if( tunfd != -1 )
      {
        close(tunfd);
      }

      // Run the interface configuration script.
      status = tspSetupInterface(c, t);
      exit(status);
    }
    else
    {
      // Parent processing
      int s = 0;

      // Wait for child process to exit.
      Display( LOG_LEVEL_3, ELInfo, "tspStartLocal", GOGO_STR_WAITING_FOR_SETUP_SCRIPT );
      if( wait(&s) != pid )
      {
        // Error occured: we have no other child
        Display( LOG_LEVEL_1, ELError, "tspStartLocal", GOGO_STR_ERR_WAITING_SCRIPT );
        status = make_status(CTX_TUNINTERFACESETUP, ERR_INTERFACE_SETUP_FAILED);;
        break;
      }

      // Check if process waited upon has exited.
      if( !WIFEXITED(s) )
      {
        // Error: child has not exited properly. Maybe killed ?
        Display( LOG_LEVEL_1, ELError, "tspStartLocal", STR_GEN_SCRIPT_EXEC_FAILED );
        status = make_status(CTX_TUNINTERFACESETUP, ERR_INTERFACE_SETUP_FAILED);;
        break;
      }

      // Check child exit code.
      status = WEXITSTATUS(s);
      if( status_number(status) != SUCCESS )
      {
        // Error in script processing.
        break;
      }
    }

    // Retrieve keepalive inteval, if found in tunnel parameters.
    if( t->keepalive_interval != NULL )
    {
      ka_interval = atoi(t->keepalive_interval);
    }


    // Start the tunnel loop, depending on tunnel mode
    //
    if( strcasecmp(t->type, STR_CONFIG_TUNNELMODE_V6UDPV4) == 0 )
    {
      status = TunMainLoop( tunfd, socket, c->keepalive,
                            ka_interval, t->client_address_ipv6,
                            t->keepalive_address );

      /* We got out of main loop = keepalive timeout || tunnel error || end */
      tspClose(socket, nt);
    }
    else if( strcasecmp(t->type, STR_CONFIG_TUNNELMODE_V6V4) == 0 )
    {
      memset( &tun_loop_cfg, 0x00, sizeof(TUNNEL_LOOP_CONFIG) );
      tun_loop_cfg.ka_interval  = ka_interval;
      tun_loop_cfg.ka_src_addr  = t->client_address_ipv6;
      tun_loop_cfg.ka_dst_addr  = t->keepalive_address;
      tun_loop_cfg.sa_family    = AF_INET6;
      tun_loop_cfg.tun_lifetime = 0;

      status = tspPerformTunnelLoop( &tun_loop_cfg );
    }
    /* v4v6 not supported yet
    if (strcasecmp(t->type, STR_CONFIG_TUNNELMODE_V4V6) == 0 )
    {
      TUNNEL_LOOP_CONFIG tun_loop_cfg;

      memset( &tun_loop_cfg, 0x00, sizeof(TUNNEL_LOOP_CONFIG) );
      tun_loop_cfg.ka_interval  = keepalive_interval;
      tun_loop_cfg.ka_src_addr  = t->client_address_ipv4;
      tun_loop_cfg.ka_dst_addr  = t->keepalive_address;
      tun_loop_cfg.sa_family    = AF_INET;
      tun_loop_cfg.tun_lifetime = 0;

      status = tspPerformTunnelLoop( &tun_loop_cfg );
    }
    */

    break; // END of DUMMY loop.
  }

  // Cleanup: Close tunnel descriptor, if it was opened.
  if( tunfd != -1 )
  {
    // The tunnel file descriptor should be closed before attempting to tear
    // down the tunnel. Destruction of the tunnel interface may fail if
    // descriptor is not closed.
    close( tunfd );
  }

  // Cleanup: Handle tunnel teardown.
  tspTearDownTunnel( c, t );


  return status;
}
