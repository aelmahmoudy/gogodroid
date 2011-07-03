/*
---------------------------------------------------------------------------
 $Id: tsp_tun.c,v 1.1 2009/11/20 16:53:26 jasminko Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2006 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
  

  Special thanks to George Koehler and Urs Breinlinger for their
  contribution.
---------------------------------------------------------------------------
*/

/* OpenBSD */

/*
 * OpenBSD is here exactly like FreeBSD, except that there is no
 * TUNSIFHEAD or TUNSDEBUG. OpenBSD does not have TUNSIFHEAD or TUNSLMODE,
 * but always behaves like TUNSIFHEAD.
 *
 * When writing packets to the tun device, OpenBSD wants them to start
 * with AF_INET6 in network byte order.
 */

#include "platform.h"
#include "gogoc_status.h"

#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <net/if.h>
#include <net/if_tun.h>
#include <sys/ioctl.h>

#include "tsp_tun.h"
#include "tsp_client.h"
#include "net_ka.h"
#include "log.h"
#include "hex_strings.h"
#include "lib.h"
#include "config.h"


// ---------------------------------------------------------------------------
/* Get the name of the tun device using file descriptor */
void TunName( int tunfd, char* name, size_t name_len )
{
  struct stat sbuf;
  char *internal_buffer;

  if( fstat(tunfd,&sbuf) != -1 )
  {
    internal_buffer = devname(sbuf.st_rdev, S_IFCHR);
    strncpy(name, internal_buffer, name_len);
  }
}


// ---------------------------------------------------------------------------
/* Initialize tun interface */
int TunInit(char *tun_device)
{
  int tunfd;
  int ifmode = IFF_POINTOPOINT | IFF_MULTICAST;
  char iftun[128];

  strcpy(iftun,"/dev/");
  strcat(iftun, tun_device);

  tunfd = open(iftun,O_RDWR);
  if (tunfd == -1)
  {
    Display(LOG_LEVEL_1, ELError, "TunInit", GOGO_STR_ERR_OPEN_TUN_DEV, iftun);
    return(-1);
  }

  if ((ioctl(tunfd,TUNSIFMODE,&ifmode) == -1))
  {
    close(tunfd);
    Display(LOG_LEVEL_1, ELError, "TunInit", GOGO_STR_ERR_CONFIG_TUN_DEV, iftun);
    return(-1);
  }

  return tunfd;
}

// ---------------------------------------------------------------------------
gogoc_status TunMainLoop(int tunfd, pal_socket_t Socket, tBoolean keepalive,
                        int keepalive_interval, char *local_address_ipv6, char *keepalive_address)
{
  fd_set rfds;
  int count, maxfd, ret;
  char bufin[2048];
  char bufout[2048];
  struct timeval timeout;
  void* p_ka_engine = NULL;
  ka_status_t ka_status;
  ka_ret_t ka_ret;
  gogoc_status status;
  int ongoing = 1;

  keepalive = (keepalive_interval != 0) ? TRUE : FALSE;

  if( keepalive == TRUE )
  {
    // Initialize the keepalive engine.
    ka_ret = KA_init( &p_ka_engine, keepalive_interval * 1000,
                      local_address_ipv6, keepalive_address, AF_INET6 );
    if( ka_ret != KA_SUCCESS )
    {
      return make_status(CTX_TUNNELLOOP, ERR_KEEPALIVE_ERROR);
    }

    // Start the keepalive loop(thread).
    ka_ret = KA_start( p_ka_engine );
    if( ka_ret != KA_SUCCESS )
    {
      KA_destroy( &p_ka_engine );
      return make_status(CTX_TUNNELLOOP, ERR_KEEPALIVE_ERROR);
    }
  }


  // tun device wants bufin[0]...[3] to contain AF_INET6
  ((u_int32_t *)bufin)[0] = htonl(AF_INET6);


  // Data send loop.
  while( ongoing == 1 )
  {
    // initialize the status.
    status = STATUS_SUCCESS_INIT;

    if( tspCheckForStopOrWait( 0 ) != 0 )
    {
      // We've been notified to stop.
      ongoing = 0;
    }

    if( keepalive == TRUE )
    {
      // Check if we're stopping.
      if( ongoing == 0 )
      {
        // Stop keepalive engine.
        KA_stop( p_ka_engine );
      }

      // Query the keepalive status.
      ka_status = KA_qry_status( p_ka_engine );
      switch( ka_status )
      {
      case KA_STAT_ONGOING:
      case KA_STAT_FIN_SUCCESS:
        break;

      case KA_STAT_FIN_TIMEOUT:
        KA_stop( p_ka_engine );
        status = make_status(CTX_TUNNELLOOP, ERR_KEEPALIVE_TIMEOUT);
        break;

      case KA_STAT_INVALID:
      case KA_STAT_FIN_ERROR:
      default:
        KA_stop( p_ka_engine );
        status = make_status(CTX_TUNNELLOOP, ERR_KEEPALIVE_ERROR);
        break;
      }

      // Reinit select timeout variable; select modifies it.
      // Use 500ms because we need to re-check keepalive status.
      timeout.tv_sec = 0;
      timeout.tv_usec = 500000;    // 500 milliseconds.
    }
    else
    {
      // Reinit select timeout variable; select modifies it.
      timeout.tv_sec = 7 * 24 * 60 * 60 ; // one week
      timeout.tv_usec = 0;
    }


    // Check if we're normal.
    if( status_number(status) != SUCCESS  ||  ongoing != 1 )
    {
      goto done;
    }

    FD_ZERO(&rfds);
    FD_SET(tunfd,&rfds);
    FD_SET(Socket,&rfds);

    maxfd = tunfd>Socket?tunfd:Socket;

    ret = select(maxfd+1,&rfds,0,0,&timeout);
    if (ret > 0)
    {
      if( FD_ISSET(tunfd, &rfds) )
      {
        /* data sent through udp tunnel */
        ioctl(tunfd, FIONREAD, &count);
        if (count > sizeof(bufout))
        {
          Display(LOG_LEVEL_1, ELError, "TunMainLoop", STR_NET_FAIL_TUN_DEV_BUFSMALL);
          status = make_status(CTX_TUNNELLOOP, ERR_TUNNEL_IO);
          goto done;
        }
        if (read(tunfd, bufout, count) != count)
        {
          Display(LOG_LEVEL_1, ELError, "TunMainLoop", STR_NET_FAIL_R_TUN_DEV);
          status = make_status(CTX_TUNNELLOOP, ERR_TUNNEL_IO);
          goto done;
        }

        if (send(Socket, bufout+4, count-4, 0) != count-4)
        {
          Display(LOG_LEVEL_1, ELError, "TunMainLoop", STR_NET_FAIL_W_SOCKET);
          status = make_status(CTX_TUNNELLOOP, ERR_TUNNEL_IO);
          goto done;
        }
      }

      if(FD_ISSET(Socket,&rfds))
      {
        // Data received through UDP tunnel.
        count=recvfrom(Socket,bufin+4,2048 - 4,0,NULL,NULL);
        if (write(tunfd, bufin, count + 4) != count + 4)
        {
          Display(LOG_LEVEL_1, ELError, "TunMainLoop", STR_NET_FAIL_W_TUN_DEV);
          status = make_status(CTX_TUNNELLOOP, ERR_TUNNEL_IO);
          goto done;
        }
      }
    }
  }

  /* Normal loop end */
  status = STATUS_SUCCESS_INIT;

done:
  if( keepalive == TRUE )
  {
    KA_destroy( &p_ka_engine );
  }

  return status;
}

