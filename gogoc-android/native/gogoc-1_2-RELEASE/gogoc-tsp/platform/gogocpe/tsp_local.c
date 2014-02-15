/*
-----------------------------------------------------------------------------
 $Id: tsp_local.c,v 1.1 2010/03/07 19:39:09 carl Exp $
-----------------------------------------------------------------------------
This source code copyright (c) gogo6 Inc. 2002-2007.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with gogo6 Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
-----------------------------------------------------------------------------
*/

/* LINUX */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "platform.h"
#include "gogoc_status.h"

#include "config.h"         /* tConf */
#include "xml_tun.h"        /* tTunnel */
#include "net.h"            /* net_tools_t */
#include "tsp_net.h"        /* tspClose */

#include "log.h"            // Display
#include "hex_strings.h"    // Various string constants

#include "tsp_tun.h"        // linux tun support
#include "tsp_client.h"     // tspSetupInterfaceLocal()
#include "tsp_setup.h"      // tspSetupInterface()
#include "tsp_tun_mgt.h"    // tspPerformTunnelLoop()

#include <gogocmessaging/gogoc_c_wrapper.h>

#ifdef HACCESS
#include "haccess.h"
#endif

#define OPENWRT_MESSAGE_FILE "/tmp/gogoc-status.log"

/* these globals are defined by US used by alot of things in  */

char *FileName  = "gogoc.conf";
char *ScriptInterpretor = "/bin/sh";
char *ScriptExtension = "sh";
char *ScriptDir = NULL;
char *TspHomeDir = "/usr/local/etc/gogoc";
char DirSeparator = '/';

int indSigHUP = 0;    // Set to 1 when HUP signal is trapped.

// implementation for the openwrt platform
//
// Update a text file
// for the GUI to show
//

static void openwrt_update_status_info()
{
  FILE *f_config;

  // Open the status file for
  // total overwrite and destruction
  //
  f_config = fopen(OPENWRT_MESSAGE_FILE, "w");

  if (f_config == NULL)
    return;

  switch (gStatusInfo.eStatus) {
    case GOGOC_CLISTAT__DISCONNECTEDIDLE:
      fprintf(f_config, "The tunnel service is disconnected, but idle.\n");
    break;
    case GOGOC_CLISTAT__DISCONNECTEDERROR:
      fprintf(f_config, "The tunnel service is disconnected for the following reason: %s.\n", get_mui_string(gStatusInfo.nStatus));
    break;
    case GOGOC_CLISTAT__CONNECTING:
      fprintf(f_config, "The tunnel service is connecting...\n");
    break;
    case GOGOC_CLISTAT__CONNECTED:
      fprintf(f_config, "The tunnel service is connected; the routing prefix is %s.\n", gTunnelInfo.szDelegatedPrefix);
    break;
  }

  fclose(f_config);
  return;
}

error_t send_status_info( void ) {
  openwrt_update_status_info();
  return GOGOCM_UIS__NOERROR;
}

error_t send_tunnel_info( void ) {
  openwrt_update_status_info();
  return GOGOCM_UIS__NOERROR;
}

error_t send_broker_list( void ) {
  openwrt_update_status_info();
  return GOGOCM_UIS__NOERROR;
}

error_t send_haccess_status_info( void ) {
  return GOGOCM_UIS__NOERROR;
}


// --------------------------------------------------------------------------
/* Verify for ipv6 support */
//
gogoc_status tspTestIPv6Support()
{
  struct stat buf;
  if(stat("/proc/net/if_inet6",&buf) == -1)
  {
    Display(LOG_LEVEL_1,ELError,"tspTestIPv6Support",GOGO_STR_NO_IPV6_SUPPORT_FOUND);
    Display(LOG_LEVEL_1,ELError,"tspTestIPv6Support",GOGO_STR_TRY_MODPROBE_IPV6);
    return make_status(CTX_TUNINTERFACESETUP, ERR_INTERFACE_SETUP_FAILED);
  }
  Display(LOG_LEVEL_2,ELInfo,"tspTestIPv6Support",GOGO_STR_IPV6_SUPPORT_FOUND);

  return STATUS_SUCCESS_INIT;
}


// --------------------------------------------------------------------------
// linux specific to setup an env variable
//
void tspSetEnv(char *Variable, char *Value, int Flag)
{
  setenv( Variable, Value, Flag );
  Display( LOG_LEVEL_3, ELInfo, "tspSetEnv", "%s=%s", Variable, Value );
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
// Returns local address.
// tspSetupTunnel() will callback here
//
char* tspGetLocalAddress(int socket, char *buffer, int size)
{
  struct sockaddr_in6 addr; /* enough place for v4 and v6 */
  struct sockaddr_in  *addr_v4 = (struct sockaddr_in *)&addr;
  struct sockaddr_in6 *addr_v6 = (struct sockaddr_in6 *)&addr;
  socklen_t len;

  len = sizeof addr;
  if (getsockname(socket, (struct sockaddr *)&addr, &len) < 0)
  {
    Display(LOG_LEVEL_1, ELError, "TryServer", GOGO_STR_ERR_FIND_SRC_IP);
    return NULL;
  }

  if (addr.sin6_family == AF_INET6)
    return (char *)inet_ntop(AF_INET6, (const void*) &addr_v6->sin6_addr, buffer, size);
  else
    return (char *)inet_ntop(AF_INET, (const void*) &addr_v4->sin_addr, buffer, size);
}

// --------------------------------------------------------------------------
// Setup tunneling interface and any daemons
// tspSetupTunnel() will callback here.
//
#ifdef DSLITE_SUPPORT
#error DSLITE support not implemented on platform
#endif
gogoc_status tspStartLocal(int socket, tConf *c, tTunnel *t, net_tools_t *nt)
{
  TUNNEL_LOOP_CONFIG tun_loop_cfg;
  gogoc_status status = STATUS_SUCCESS_INIT;
  int ka_interval = 0;
  int tunfd = (-1);
  sint32_t redirect = 0;
#ifdef HACCESS
  int do_haccess_teardown = 0;
  int do_haccess_hide_devices = 0;
  haccess_status action_status = HACCESS_STATUS_OK;
#endif


  // Check if we got root privileges.
  if(geteuid() != 0)
  {
    // Error: we don't have root privileges.
    Display( LOG_LEVEL_1, ELError, "tspStartLocal", GOGO_STR_FATAL_NOT_ROOT_FOR_TUN );
    return make_status(CTX_TUNINTERFACESETUP, ERR_INTERFACE_SETUP_FAILED);
  }

  // Check Ipv6 support.
  Display( LOG_LEVEL_2, ELInfo, "tspStartLocal", GOGO_STR_CHECKING_LINUX_IPV6_SUPPORT );
  status = tspTestIPv6Support();
  if( status_number(status) != SUCCESS )
  {
    // Error: It seems the user does not have IPv6 support in kernel.
    return status;
  }

// Don't call daemon() for the Dongle6.
// It implies a fork, and the uClibC pthread implementation
// has a problem with pthread_create() calls after a fork.
#if 0
  // Detach from controlling terminal and run in the background.
  Display( LOG_LEVEL_3, ELInfo, "tspStartLocal", GOGO_STR_GOING_DAEMON );
  if( daemon(1,0) == -1 )
  {
    // Error: Failed to detach.
    Display( LOG_LEVEL_1, ELError, "tspStartLocal", GOGO_STR_CANT_FORK );
    return make_status(CTX_TUNINTERFACESETUP, ERR_INTERFACE_SETUP_FAILED);
  }
#endif

  // Check tunnel mode.
  if( strcasecmp(t->type, STR_CONFIG_TUNNELMODE_V4V6) == 0 )
  {
    // V4V6 tunnel mode is not supported on this platform.
    Display( LOG_LEVEL_1, ELError, "tspStartLocal", GOGO_STR_NO_V4V6_ON_PLATFORM );
    return make_status(CTX_TUNINTERFACESETUP, ERR_INTERFACE_SETUP_FAILED);
  }
  else if( strcasecmp(t->type, STR_CONFIG_TUNNELMODE_V6UDPV4) == 0 )
  {
    // When using V6UDPV4 encapsulation, open the TUN device.
    tunfd = TunInit(c->if_tunnel_v6udpv4);
    if( tunfd == -1 )
    {
      // Error: Failed to open TUN device.
      Display( LOG_LEVEL_1, ELError, "tspStartLocal", STR_MISC_FAIL_TUN_INIT );
      return make_status(CTX_TUNINTERFACESETUP, ERR_INTERFACE_SETUP_FAILED);
    }

    status = TunRedirect(tunfd, socket, nt, &redirect);
    if( status_number(status) != SUCCESS )
    {
        goto done;
    }
  }

  // now, run the template script
  //
  status = tspSetupInterface(c, t);
  if( status_number(status) != SUCCESS )
  {
    goto done;
  }

  gStatusInfo.eStatus = GOGOC_CLISTAT__CONNECTED;
  gStatusInfo.nStatus = GOGOCM_UIS__NOERROR;
  send_status_info();

#ifdef HACCESS
  /* If both HACCESS features are disabled, we just skip this. */
  if ((c->haccess_web_enabled == TRUE) || (c->haccess_proxy_enabled == TRUE))
  {
    /* Execute the setup action after the template script has run the system is setup. */
    action_status = haccess_exec_action(HACCESS_ACTION_SETUP, c, t);

    do_haccess_teardown = 1;

    if (action_status != HACCESS_STATUS_OK)
    {
      /* Could not setup HACCESS, so all features fail. */
      haccess_messaging_set_status_web(HACCESS_FEATSTTS_ERROR);
      haccess_messaging_set_status_proxy(HACCESS_FEATSTTS_ERROR);

      send_haccess_status_info();

      status = make_status(CTX_TUNINTERFACESETUP, ERR_HACCESS_SETUP);
      goto teardown;
    }

    /* HACCESS setup ok, features up successfully. */
    haccess_messaging_set_status_web(HACCESS_FEATSTTS_SUCCESS);
    haccess_messaging_set_status_proxy(HACCESS_FEATSTTS_SUCCESS);

    send_haccess_status_info();

    if (c->haccess_proxy_enabled == TRUE)
    {
      /* If proxying is enabled, also expose the devices (proxy and DDNS). */
      action_status = haccess_exec_action(HACCESS_ACTION_EXPOSE_DEVICES, c, t);

      do_haccess_hide_devices = 1;

      if (action_status != HACCESS_STATUS_OK)
      {
        /* Proxy feature failed. */
        haccess_messaging_set_status_proxy(HACCESS_FEATSTTS_ERROR);

        send_haccess_status_info();

        status = make_status(CTX_TUNINTERFACESETUP, ERR_HACCESS_EXPOSE_DEVICES);
        goto teardown;
      }
    }

  }
#endif


  // Retrieve keepalive inteval, if found in tunnel parameters.
  if( t->keepalive_interval != NULL )
  {
    ka_interval = atoi(t->keepalive_interval);
  }

  // Start the tunnel loop, depending on tunnel mode
  //
  if( strcasecmp(t->type, STR_CONFIG_TUNNELMODE_V6UDPV4) == 0 )
  {
    status = TunMainLoop( tunfd, socket, redirect, c->keepalive,
                          ka_interval, t->client_address_ipv6,
                          t->keepalive_address);
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

teardown:
#ifdef HACCESS
  if (do_haccess_hide_devices)
  {
    /* We need to send DDNS deletion requests to remove */
    /* the devices from the DNS servers. */
    action_status = haccess_exec_action(HACCESS_ACTION_HIDE_DEVICES, c, t);

    if (action_status != HACCESS_STATUS_OK)
    {
      Display(LOG_LEVEL_1, ELError, "tspStartLocal", HACCESS_LOG_PREFIX_ERROR GOGO_STR_HACCESS_ERR_CANT_DO_HIDE_DEVICES);
    }
  }

  if (do_haccess_teardown)
  {
    /* The teardown action must be executed when we disconnect */
    /* to clean up what the setup action did. */
    action_status = haccess_exec_action(HACCESS_ACTION_TEARDOWN, c, t);

    if (action_status != HACCESS_STATUS_OK)
    {
      Display(LOG_LEVEL_1, ELError, "tspStartLocal", HACCESS_LOG_PREFIX_ERROR GOGO_STR_HACCESS_ERR_CANT_DO_TEARDOWN);
    }
  }
#endif

  // Cleanup: Handle tunnel teardown.
  tspTearDownTunnel( c, t );

done:
  if (tunfd >= 0)
  {
    close(tunfd);
  }

  return status;
}
