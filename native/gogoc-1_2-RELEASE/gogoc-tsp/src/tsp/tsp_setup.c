/*
-----------------------------------------------------------------------------
 $Id: tsp_setup.c,v 1.2 2010/03/07 20:14:32 carl Exp $
-----------------------------------------------------------------------------
Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"
#include "gogoc_status.h"       // Error codes

#include "tsp_setup.h"
#include "tsp_client.h"

#include "config.h"       // tConf
#include "xml_tun.h"      // tTunnel
#include "log.h"          // Display()
#include "hex_strings.h"  // Strings for Display()
#include "lib.h"          // IsAll, IPv4Addr, IPv6Addr, IPAddrAny, Numeric.

// gogoCLIENT Messaging Subsystem.
#include <gogocmessaging/gogoc_c_wrapper.h>

#define TSP_OPERATION_CREATETUNNEL    "TSP_TUNNEL_CREATION"
#define TSP_OPERATION_TEARDOWNTUNNEL  "TSP_TUNNEL_TEARDOWN"


/*  Should be defined in platform.h  */
#ifndef SCRIPT_TMP_FILE
#error "SCRIPT_TMP_FILE is not defined in platform.h"
#endif


#ifdef ANDROID
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/route.h>

struct in6_ifreq {
  struct in6_addr ifr6_addr;
  __u32           ifr6_prefixlen;
  int             ifr6_ifindex; 
};

struct in6_rtmsg {
  struct in6_addr rtmsg_dst;
  struct in6_addr rtmsg_src;
  struct in6_addr rtmsg_gateway;
  __u32           rtmsg_type;
  __u16           rtmsg_dst_len;
  __u16           rtmsg_src_len;
  __u32           rtmsg_metric;
  unsigned long   rtmsg_info;
  __u32           rtmsg_flags;
  int             rtmsg_ifindex;
};

static int ifup(const char *name, const char *host, int prefix, int mtu)
{
  int retcode = -1;
  struct ifreq ifr;
  struct in6_ifreq ifr6;

  int sock;
  if ((sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
    fprintf(stderr, "Cannot Create Socket\n");
    return -1;
  }

  strncpy(ifr.ifr_name, name, IFNAMSIZ);
  if (ioctl(sock, SIOGIFINDEX, &ifr) < 0) {
    perror("SIOGIFINDEX");
    goto error;
  }

  ifr6.ifr6_ifindex = ifr.ifr_ifindex;
  ifr6.ifr6_prefixlen = prefix;
  inet_pton(AF_INET6, host, &ifr6.ifr6_addr);

  if(ioctl(sock, SIOCSIFADDR, &ifr6) < 0) {
    perror("SIOCSIFADDR");
    goto error;
  }

  if (ioctl(sock, SIOCGIFFLAGS, &ifr)) {
    perror("SIOCGIFFLAGS");
    goto error;
  }

  ifr.ifr_flags |= IFF_UP;
  ifr.ifr_flags |= IFF_RUNNING;
  if (ioctl(sock, SIOCSIFFLAGS, &ifr)) {
    perror("SIOCSIFFLAGS");
    goto error;
  }

  ifr.ifr_mtu = mtu;
  if(ioctl(sock, SIOCSIFMTU, &ifr) < 0) {
    perror("SIOCSIFMTU");
    goto error;
  }

  retcode = 0;

error:
  close(sock);
  return retcode;
}

#define IFINET6 "/proc/net/if_inet6"
static int ifdown(char *name, int down) {
  int sock;
  FILE *f;
  char addr6[40], devname[20];
  int plen, scope, dad_status, if_idx = -1;
  char addr6p[8][5];
  struct in6_ifreq ifr6;
  struct ifreq ifr;

  if ((sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
    fprintf(stderr, "Cannot Create Socket\n");
    return -1;
  }
  f = fopen(IFINET6, "r");
  if (f == NULL)
    return -1;
  while (fscanf(f, "%4s%4s%4s%4s%4s%4s%4s%4s %08x %02x %02x %02x %20s\n",
        addr6p[0], addr6p[1], addr6p[2], addr6p[3],
        addr6p[4], addr6p[5], addr6p[6], addr6p[7],
        &if_idx, &plen, &scope, &dad_status, devname) != EOF) {
    if (!strcmp(devname, name)) {
      sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s",
          addr6p[0], addr6p[1], addr6p[2], addr6p[3],
          addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
      ifr6.ifr6_ifindex = if_idx;
      ifr6.ifr6_prefixlen = plen;
      inet_pton(AF_INET6, addr6, &ifr6.ifr6_addr);
      if(ioctl(sock, SIOCDIFADDR, &ifr6) < 0) {
        perror("SIOCDIFADDR");
      }
    }
  }
  if (if_idx != -1 && down) {
    strncpy(ifr.ifr_name, name, IFNAMSIZ);
    if (ioctl(sock, SIOCGIFFLAGS, &ifr)) {
      perror("SIOCGIFFLAGS");
    }

    ifr.ifr_flags &= ~IFF_UP;
    if (ioctl(sock, SIOCSIFFLAGS, &ifr)) {
      perror("SIOCSIFFLAGS");
    }
  }
  close(sock);
  fclose(f);
  return 0;
}

static int route(const char *name, const char *host, int prefix, int action)
{
  int sock;
  int retcode = -1;
  struct ifreq ifr;
  struct in6_rtmsg rt;

  memset(&rt, 0, sizeof(rt));
  inet_pton(AF_INET6, host, &rt.rtmsg_dst);

  rt.rtmsg_dst_len = prefix;
  rt.rtmsg_flags = ((prefix == 128) ? (RTF_UP|RTF_HOST) : RTF_UP);
  rt.rtmsg_metric = 1;

  if ((sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
    fprintf(stderr, "Cannot Create Socket\n");
    return -1;
  }

  if (name && strncpy(ifr.ifr_name, name, IFNAMSIZ)
    && ioctl(sock, SIOGIFINDEX, &ifr) == 0) {
    rt.rtmsg_ifindex = ifr.ifr_ifindex;
  }

  if (ioctl(sock, action ? SIOCADDRT : SIOCDELRT, &rt) < 0) {
    perror(action ? "SIOCADDRT" : "SIOCDELRT");
  } else {
    retcode = 0;
  }
  close(sock);

  return retcode;
}

static void addroute(const char *interface) {
  route(interface, "::", 0, 1);
  route(interface, "2000::", 3, 1);
}

static void delroute() {
  route(NULL, "::", 0, 0);
  route(NULL, "2000::", 3, 0);
}

#ifndef IP_DF
#define IP_DF 0x4000
#endif

#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE 0x89F0
#endif
#define SIOCADDTUNNEL (SIOCDEVPRIVATE + 1)
#define SIOCDELTUNNEL (SIOCDEVPRIVATE + 2)
struct ip_tunnel_parm {
  char        name[IFNAMSIZ];
  int         link;
  __be16      i_flags;
  __be16      o_flags;
  __be32      i_key;
  __be32      o_key;
 struct iphdr iph;
};

static int del_tunnel(const char *name)
{
  int sock, code;
  struct ifreq ifr;
  struct ip_tunnel_parm p;

  if (!name || !name[0]) {
    return -EINVAL;
  }

  memset(&p, 0, sizeof(p));
  p.iph.version = 4;
  p.iph.ihl = 5;
  p.iph.frag_off = htons(IP_DF);
  strncpy(p.name, name, IFNAMSIZ);

  strncpy(ifr.ifr_name, name, IFNAMSIZ);
  ifr.ifr_ifru.ifru_data = &p;

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  code = ioctl(sock, SIOCDELTUNNEL, &ifr);
  if (code) {
    fprintf(stderr, "delete tunnel %s failed: %s\n",
        ifr.ifr_name, strerror(errno));
  }
  close(sock);
  return code;
}

static int add_tunnel(const char *name, const char *remote, const char *local)
{
  int sock, code;
  struct ifreq ifr;
  struct ip_tunnel_parm p;

  if (!name || !name[0] || !remote || !remote[0]) {
    return -EINVAL;
  }

  memset(&p, 0, sizeof(p));
  p.iph.version = 4;
  p.iph.ihl = 5;
  p.iph.frag_off = htons(IP_DF);
  p.iph.protocol = IPPROTO_IPV6;
  p.iph.ttl = 64;
  inet_pton(AF_INET, remote, &(p.iph.daddr));
  if (local && local[0]) {
    inet_pton(AF_INET, local, &(p.iph.saddr));
  }
  strncpy(p.name, name, IFNAMSIZ);

  strncpy(ifr.ifr_name, "sit0", IFNAMSIZ);
  ifr.ifr_ifru.ifru_data = &p;
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  code = ioctl(sock, SIOCADDTUNNEL, &ifr);
  if (code) {
    fprintf(stderr, "add tunnel %s failed: %s\n",
        ifr.ifr_name, strerror(errno));
  }
  close(sock);
  return code;
}
#else
/* Execute cmd and send output to log subsystem */
sint32_t execScript( const char *cmd )
{
  char buf[1024];
  FILE* f_log;
  sint32_t retVal;

  // Run the command.
  memset( buf, 0, sizeof(buf) );
  pal_snprintf( buf, sizeof(buf), "%s > %s", cmd, SCRIPT_TMP_FILE );
  retVal = pal_system( buf );

  // Open resulting output file.
  f_log = fopen( SCRIPT_TMP_FILE, "r" );
  if( f_log == NULL )
  {
    Display( LOG_LEVEL_1, ELError, "execScript", GOGO_STR_CANT_OPEN_TMP_FILE SCRIPT_TMP_FILE );
    return -1;
  }

  // Loop on the output file, and log the contents.
  while( !feof( f_log ) )
  {
    if( fgets( buf, sizeof(buf), f_log ) != NULL )
    {
      Display( LOG_LEVEL_MAX, ELInfo, "execScript", "%s", buf );
    }
  }
  // Close file
  fclose( f_log );

  // Remove command output.
  pal_unlink( SCRIPT_TMP_FILE );

  return retVal;
}
#endif


// --------------------------------------------------------------------------
// This function validates the information found in the tunnel information
// structure.
// Returns number of errors found. 0 is successful validation.
//
sint32_t validate_tunnel_info( const tTunnel* pTunnelInfo )
{
  sint32_t err_num = 0;


  if( !IsAll(IPv4Addr, pTunnelInfo->client_address_ipv4) )
  {
    Display(LOG_LEVEL_1, ELError, "validate_tunnel_info", GOGO_STR_BAD_CLIENT_IPV4_RECVD);
    err_num++;
  }

  if( !IsAll(IPv6Addr, pTunnelInfo->client_address_ipv6) )
  {
    Display(LOG_LEVEL_1, ELError, "validate_tunnel_info", GOGO_STR_BAD_CLIENT_IPV6_RECVD);
    err_num++;
  }

  if( pTunnelInfo->client_dns_server_address_ipv6 != NULL )
  {
    if( !IsAll(IPv6Addr, pTunnelInfo->client_dns_server_address_ipv6) )
    {
      Display(LOG_LEVEL_1, ELError, "validate_tunnel_info", GOGO_STR_BAD_CLIENT_DNS_IPV6_RECVD);
      err_num++;
    }
  }

  if( !IsAll(IPv4Addr, pTunnelInfo->server_address_ipv4) )
  {
    Display(LOG_LEVEL_1, ELError, "validate_tunnel_info", GOGO_STR_BAD_SERVER_IPV4_RECVD);
    err_num++;
  }

  if( !IsAll(IPv6Addr, pTunnelInfo->server_address_ipv6) )
  {
    Display(LOG_LEVEL_1, ELError, "validate_tunnel_info", GOGO_STR_BAD_SERVER_IPV6_RECVD);
    err_num++;
  }

  // If prefix information is found, validate it.
  if( pTunnelInfo->prefix != NULL )
  {
    if( !IsAll(IPAddrAny, pTunnelInfo->prefix) )
    {
      Display(LOG_LEVEL_1, ELError, "validate_tunnel_info", GOGO_STR_BAD_SERVER_PREFIX_RECVD);
      err_num++;
    }

    if( !IsAll(Numeric, pTunnelInfo->prefix_length) )
    {
      Display(LOG_LEVEL_1, ELError, "validate_tunnel_info", GOGO_STR_BAD_PREFIX_LEN_RECVD);
      err_num++;
    }
  }

  return err_num;
}


// --------------------------------------------------------------------------

void set_tsp_env_variables( const tConf* pConfig, const tTunnel* pTunnelInfo )
{
  char buffer[8];

  // Specify log verbosity (MAXIMAL).
  pal_snprintf( buffer, sizeof buffer, "%d", LOG_LEVEL_MAX );
  tspSetEnv("TSP_VERBOSE", buffer, 1);

  // Specify gogoCLIENT installation directory.
  tspSetEnv("TSP_HOME_DIR", TspHomeDir, 1);

  // Specify the tunnel mode.
  tspSetEnv("TSP_TUNNEL_MODE", pTunnelInfo->type, 1);

  // Specify host type {router, host}
  tspSetEnv("TSP_HOST_TYPE", pConfig->host_type, 1);

  // Specify tunnel interface, for setup.
  if (pal_strcasecmp(pTunnelInfo->type, STR_XML_TUNNELMODE_V6V4) == 0 )
  {
    tspSetEnv("TSP_TUNNEL_INTERFACE", pConfig->if_tunnel_v6v4, 1);
    gTunnelInfo.eTunnelType = TUNTYPE_V6V4;
  }
  else if (pal_strcasecmp(pTunnelInfo->type, STR_XML_TUNNELMODE_V6UDPV4) == 0 )
  {
    tspSetEnv("TSP_TUNNEL_INTERFACE", pConfig->if_tunnel_v6udpv4, 1);
    gTunnelInfo.eTunnelType = TUNTYPE_V6UDPV4;
  }
#ifdef V4V6_SUPPORT
  else if (pal_strcasecmp(pTunnelInfo->type, STR_XML_TUNNELMODE_V4V6) == 0 )
  {
    tspSetEnv("TSP_TUNNEL_INTERFACE", pConfig->if_tunnel_v4v6, 1);
    gTunnelInfo.eTunnelType = TUNTYPE_V4V6;
  }
#endif /* V4V6_SUPPORT */

  // Specify what interface will be used for routing advertizement,
  // if enabled.
  tspSetEnv("TSP_HOME_INTERFACE", pConfig->if_prefix, 1);

  // Specify local endpoint IPv4 address
  tspSetEnv("TSP_CLIENT_ADDRESS_IPV4", pTunnelInfo->client_address_ipv4, 1);
  gTunnelInfo.szIPV4AddrLocalEndpoint = pTunnelInfo->client_address_ipv4;

  // Specify local endpoint IPv6 address
  tspSetEnv("TSP_CLIENT_ADDRESS_IPV6", pTunnelInfo->client_address_ipv6, 1);
  gTunnelInfo.szIPV6AddrLocalEndpoint = pTunnelInfo->client_address_ipv6;

  // Specify client dns IPv6 address
  tspSetEnv("TSP_CLIENT_DNS_ADDRESS_IPV6", pTunnelInfo->client_dns_server_address_ipv6, 1);
  gTunnelInfo.szIPV6AddrDns = pTunnelInfo->client_dns_server_address_ipv6;

  // Specify local endpoint domain name
  if( pTunnelInfo->client_dns_name != NULL)
  {
    tspSetEnv("TSP_CLIENT_DNS_NAME", pTunnelInfo->client_dns_name, 1);
    gTunnelInfo.szUserDomain = pTunnelInfo->client_dns_name;
  }

  // Specify remote endpoint IPv4 address.
  tspSetEnv("TSP_SERVER_ADDRESS_IPV4", pTunnelInfo->server_address_ipv4, 1);
  gTunnelInfo.szIPV4AddrRemoteEndpoint = pTunnelInfo->server_address_ipv4;

  // Specify remote endpoint IPv6 address.
  tspSetEnv("TSP_SERVER_ADDRESS_IPV6", pTunnelInfo->server_address_ipv6, 1);
  gTunnelInfo.szIPV6AddrRemoteEndpoint = pTunnelInfo->server_address_ipv6;

  // Specify prefix for tunnel endpoint.
  if ((pal_strcasecmp(pTunnelInfo->type, STR_XML_TUNNELMODE_V6V4) == 0) ||
      (pal_strcasecmp(pTunnelInfo->type, STR_XML_TUNNELMODE_V6UDPV4) == 0))
    tspSetEnv("TSP_TUNNEL_PREFIXLEN", "128", 1);
#ifdef V4V6_SUPPORT
  else
    tspSetEnv("TSP_TUNNEL_PREFIXLEN", "32", 1);
#endif /* V4V6_SUPPORT */


  // Free and clear delegated prefix from tunnel info.
  if( gTunnelInfo.szDelegatedPrefix != NULL )
  {
    pal_free( gTunnelInfo.szDelegatedPrefix );
    gTunnelInfo.szDelegatedPrefix = NULL;
  }

  // Have we been allocated a prefix for routing advertizement..?
  if( pTunnelInfo->prefix != NULL )
  {
    char chPrefix[128];
    size_t len, sep;

    /* Compute the number of characters that are significant out of the prefix. */
    /* This is meaningful only for IPv6 prefixes; no contraction is possible for IPv4. */
    if ((pal_strcasecmp(pTunnelInfo->type, STR_XML_TUNNELMODE_V6V4) == 0) ||
        (pal_strcasecmp(pTunnelInfo->type, STR_XML_TUNNELMODE_V6UDPV4) == 0))
    {
      len = (atoi(pTunnelInfo->prefix_length) % 16) ? (atoi(pTunnelInfo->prefix_length) / 16 + 1) * 4 : atoi(pTunnelInfo->prefix_length) / 16 * 4;
      sep = (atoi(pTunnelInfo->prefix_length) % 16) ? (atoi(pTunnelInfo->prefix_length) / 16) : (atoi(pTunnelInfo->prefix_length) / 16) -1;
    }
    else
    {
      len = pal_strlen( pTunnelInfo->prefix );
      sep = 0;
    }

    memset(chPrefix, 0, 128);
    memcpy(chPrefix, pTunnelInfo->prefix, len+sep);

    // Specify delegated prefix for routing advertizement, if enabled.
    tspSetEnv("TSP_PREFIX", chPrefix, 1);
    gTunnelInfo.szDelegatedPrefix = (char*) pal_malloc( pal_strlen(chPrefix) + 10/*To append prefix_length*/ );
    strcpy( gTunnelInfo.szDelegatedPrefix, chPrefix );

    // Specify prefix length for routing advertizement, if enabled.
    tspSetEnv("TSP_PREFIXLEN", pTunnelInfo->prefix_length, 1);
    strcat( gTunnelInfo.szDelegatedPrefix, "/" );
    strcat( gTunnelInfo.szDelegatedPrefix, pTunnelInfo->prefix_length );
  }
}


// --------------------------------------------------------------------------
// Builds the template script execution path and returns the string.
//
// Returns NULL if error is detected and template execution path cannot be
// built.
//
// NOTE: This function is NOT thread safe. Callee should not retain
//       pointer returned from this function.
//
static char* get_template_script( const tConf* pConfig )
{
  static char buffer[1024] = { 0x00 };
  FILE* f_test;

  // If first run, get the required information and build template
  // script execution path.
  //
  if( buffer[0] == 0x00 )
  {
    pal_snprintf( buffer, sizeof buffer, "%s%c%s.%s", ScriptDir, DirSeparator, pConfig->template, ScriptExtension);

    f_test = fopen( buffer, "r" );
    if( f_test == NULL )
    {
      Display( LOG_LEVEL_1, ELError, "tspSetupInterface", GOGO_STR_TEMPLATE_NOT_FOUND, buffer );
      return NULL;
    }

    // Close the just opened file.
    fclose( f_test );
    memset( buffer, 0, sizeof buffer );

    // Append script interpretor to buffer, if any.
    if( ScriptInterpretor != NULL )
    {
      pal_snprintf( buffer, sizeof buffer,
               "%s \"%s%c%s.%s\"",
               ScriptInterpretor, ScriptDir, DirSeparator, pConfig->template, ScriptExtension);
    }
    else
    {
      pal_snprintf( buffer, sizeof buffer,
                "\"%s%c%s.%s\"",
                ScriptDir, DirSeparator, pConfig->template, ScriptExtension);
    }
  }

  return buffer;
}


// --------------------------------------------------------------------------
// This function will set the required environment variables that will later
// be used when invoking the template script to actually create the tunnel.
//
gogoc_status tspSetupInterface(tConf *c, tTunnel *t)
{
  gogoc_status status = STATUS_SUCCESS_INIT;
  char* template_script;


  // Perform validation on tunnel information provided by server.
  if( validate_tunnel_info(t) != 0 )
  {
    // Errors occured during verification of tunnel parameters.
    Display( LOG_LEVEL_1, ELError, "tspSetupInterface", STR_TSP_ERRS_TUN_PARAM_FROM_SERVER );
    return make_status(CTX_TUNINTERFACESETUP, ERR_BAD_TUNNEL_PARAM);
  }


  // Specify TSP Operation: Tunnel Creation.
  tspSetEnv("TSP_OPERATION", TSP_OPERATION_CREATETUNNEL, 1 );

  // Set environment variable for script execution.
  set_tsp_env_variables( c, t );


  // Do some platform-specific stuff before tunnel setup script is launched.
  // The "tspSetupInterfaceLocal" is defined in tsp_local.c in every platform.
  if( tspSetupInterfaceLocal( c, t ) != 0 )
  {
    // Errors occured during setup of interface.
    return make_status(CTX_TUNINTERFACESETUP, ERR_INTERFACE_SETUP_FAILED);
  }


#ifndef ANDROID
  // Get interface configuration script command string.
  template_script = get_template_script( c );
  if( template_script == NULL )
  {
    // Failed to get filename/directory.
    return make_status(CTX_TUNINTERFACESETUP, ERR_INVAL_CFG_FILE);
  }


  // ---------------------------------------------------------------
  // Run the interface configuration script to bring the tunnel up.
  // ---------------------------------------------------------------
  Display( LOG_LEVEL_2, ELInfo, "tspSetupInterface", STR_GEN_EXEC_CFG_SCRIPT, template_script );
  if( execScript( template_script ) != 0 )
  {
    // Error executing script.
    Display(LOG_LEVEL_1, ELError, "tspSetupInterface", STR_GEN_SCRIPT_EXEC_FAILED);
    return make_status(CTX_TUNINTERFACESETUP, ERR_INTERFACE_SETUP_FAILED);
  }
  Display(LOG_LEVEL_2, ELInfo, "tspSetupInterface", STR_GEN_SCRIPT_EXEC_SUCCESS);
#else
  {
    char *interface = NULL;
    Display(LOG_LEVEL_2, ELInfo, "tspSetupInterface", "...");
    if (pal_strcasecmp(t->type, STR_XML_TUNNELMODE_V6V4) == 0)
    {
      interface = c->if_tunnel_v6v4;
      del_tunnel(interface);
      sleep(1);
      add_tunnel(interface, t->server_address_ipv4, NULL);
      Display(LOG_LEVEL_2, ELInfo, "tspSetupInterface", "v6v4");
    } else if (pal_strcasecmp(t->type, STR_XML_TUNNELMODE_V6UDPV4) == 0) {
      interface = c->if_tunnel_v6udpv4;
      Display(LOG_LEVEL_2, ELInfo, "tspSetupInterface", "v6udpv4");
    } else {
      return make_status(CTX_TUNINTERFACESETUP, ERR_INTERFACE_SETUP_FAILED);
    }
    Display(LOG_LEVEL_2, ELInfo, "tspSetupInterface", interface);
    ifdown(interface, 0);
    Display(LOG_LEVEL_2, ELInfo, "tspSetupInterface", t->client_address_ipv6);
    ifup(interface, t->client_address_ipv6, 128, 1280);
    delroute();
    addroute(interface);
  }
#endif


  // Display a resume of the configured settings.
  Display(LOG_LEVEL_2, ELInfo, "tspSetupInterface", GOGO_STR_SETUP_HOST_TYPE, c->host_type);
  Display(LOG_LEVEL_2, ELInfo, "tspSetupInterface", GOGO_STR_SETUP_TUNNEL_TYPE, t->type);
  Display(LOG_LEVEL_3, ELInfo, "tspSetupInterface", GOGO_STR_SETUP_PROXY, c->proxy_client == TRUE ? STR_LIT_ENABLED : STR_LIT_DISABLED);

  if( (pal_strcasecmp(t->type, STR_XML_TUNNELMODE_V6V4) == 0) ||
      (pal_strcasecmp(t->type, STR_XML_TUNNELMODE_V6UDPV4) == 0))
  {
    Display(LOG_LEVEL_1, ELInfo, "tspSetupInterface", GOGO_STR_YOUR_IPV6_IP_IS, t->client_address_ipv6);
    if( (t->prefix != NULL) && (t->prefix_length != NULL) )
      Display(LOG_LEVEL_1, ELInfo, "tspSetupInterface", GOGO_STR_YOUR_IPV6_PREFIX_IS, t->prefix, t->prefix_length);
    if (t->client_dns_server_address_ipv6 != NULL)
      Display(LOG_LEVEL_1, ELInfo, "tspSetupInterface", GOGO_STR_YOUR_IPV6_DNS_IS, t->client_dns_server_address_ipv6);
  }
#ifdef V4V6_SUPPORT
  else
  {
    Display(LOG_LEVEL_1, ELInfo, "tspSetupInterface", GOGO_STR_YOUR_IPV4_IP_IS, t->client_address_ipv4);
    if( (t->prefix != NULL) && (t->prefix_length != NULL) )
      Display(LOG_LEVEL_1, ELInfo, "tspSetupInterface", GOGO_STR_YOUR_IPV4_PREFIX_IS, t->prefix, t->prefix_length);
  }
#endif /* V4V6_SUPPORT */


  // Set the broker used for connection & the current time(now) for tunnel
  //   start. Then send the tunnel info through the messaging subsystem.
  gTunnelInfo.szBrokerName = c->server;
  gTunnelInfo.tunnelUpTime = pal_time(NULL);
  send_tunnel_info();

  return status;
}


// --------------------------------------------------------------------------
// This function will set the required environment variables that will later
// be used when invoking the template script to tear down the existing
// tunnel.
//
gogoc_status tspTearDownTunnel( tConf* pConf, tTunnel* pTunInfo )
{
  char* scriptName;


  // Specify TSP Operation: Tunnel Teardown.
  tspSetEnv( "TSP_OPERATION", TSP_OPERATION_TEARDOWNTUNNEL, 1 );

  // Set environment variables (They may be not set).
  set_tsp_env_variables( pConf, pTunInfo );

#ifndef ANDROID 
  // Format path to script.
  scriptName = get_template_script( pConf );
  if( scriptName == NULL )
  {
    return make_status(CTX_GOGOCTEARDOWN, ERR_INVAL_CFG_FILE);
  }

  // Run the template script to tear the tunnel down.
  Display(LOG_LEVEL_2, ELInfo, "tspTearDownTunnel", STR_GEN_EXEC_CFG_SCRIPT, scriptName );
  if( execScript( scriptName ) != 0 )
  {
    // Error executing script.
    Display(LOG_LEVEL_1, ELError, "tspTearDownTunnel", STR_GEN_SCRIPT_EXEC_FAILED );
    return make_status(CTX_GOGOCTEARDOWN, ERR_INTERFACE_SETUP_FAILED);
  }
  Display(LOG_LEVEL_2, ELInfo, "tspTearDownTunnel", STR_GEN_SCRIPT_EXEC_SUCCESS );
#else
  delroute();
  if (pal_strcasecmp(pTunInfo->type, STR_XML_TUNNELMODE_V6V4) == 0)
  {
      del_tunnel(pConf->if_tunnel_v6v4);
  } else if (pal_strcasecmp(pTunInfo->type, STR_XML_TUNNELMODE_V6UDPV4) == 0) {
      del_tunnel(pConf->if_tunnel_v6udpv4);
  }
#endif


  // Return script execution return code.
  return STATUS_SUCCESS_INIT;
}
