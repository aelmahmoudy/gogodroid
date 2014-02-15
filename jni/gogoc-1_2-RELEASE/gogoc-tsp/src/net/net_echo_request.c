/*
-----------------------------------------------------------------------------
 $Id: net_echo_request.c,v 1.1 2009/11/20 16:53:39 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"
#include "gogoc_status.h"

#include "net_rudp.h"
#include "tsp_net.h"
#include "net.h"
#include "log.h"
#include "hex_strings.h"
#include "tsp_redirect.h"
#include "tsp_client.h"
#include "net_echo_request.h"


/* Create a stat engine for a distance calculation thread */
rttengine_stat_t *createStatEngine()
{
  rttengine_stat_t *engine = NULL;

  /* Allocate memory */
  engine = (rttengine_stat_t *)pal_malloc(sizeof(rttengine_stat_t));
  if( engine == NULL )
  {
    return NULL;
  }

  /* And zero it */
  memset( engine, 0, sizeof(rttengine_stat_t) );

  return engine;
}

/* Create a socket and connect it to a given address */
pal_socket_t createConnectedSocket(struct addrinfo *address_info)
{
  pal_socket_t sfd;
  struct sockaddr_in sai;
  struct sockaddr_in6 sai6;
  struct sockaddr *sa = NULL;
  sint32_t sa_len = 0;

  /* Zero out the structures */
  memset(&sai, 0, sizeof(struct sockaddr_in));
  memset(&sai6, 0, sizeof(struct sockaddr_in6));

  /* It's an IPv4 address */
  if (address_info->ai_family == AF_INET)
  {
    /* Copy the information into the IPv4-specific structure */
    memcpy(&sai.sin_addr, &(((struct sockaddr_in *)address_info->ai_addr)->sin_addr), sizeof(struct in_addr));
    sai.sin_family = AF_INET;
    sai.sin_port = htons(SERVER_PORT);

    /* Set a generic pointer for the connect() call */
    sa = (struct sockaddr *)&sai;
    sa_len = sizeof(struct sockaddr_in);
  }
  /* It's an IPv6 address */
  else if (address_info->ai_family == AF_INET6)
  {
    /* Copy the information into the IPv6-specific structure */
    memcpy(&(sai6.sin6_addr), &(((struct sockaddr_in6 *)address_info->ai_addr)->sin6_addr), sizeof(struct in6_addr));
    sai6.sin6_family = AF_INET6;
    sai6.sin6_port = htons(SERVER_PORT);
#ifdef SIN6_LEN
    sai6.sin6_len = sizeof(struct sockaddr_in6);
#endif

    /* Set a generic pointer for the connect() call */
    sa = (struct sockaddr *)&sai6;
    sa_len = sizeof(struct sockaddr_in6);
  }
  else {
    return (pal_socket_t)(-1);
  }

  /* Create the socket */
  if ((sfd = pal_socket(address_info->ai_family, address_info->ai_socktype, 0)) == -1)
  {
    return (pal_socket_t)(-1);
  }

  /* And connect it using the generic pointer */
  if (connect(sfd, sa, sa_len) == -1) {
    return (pal_socket_t)(-1);
  }

  return sfd;
}

/* Destroy a socket */
sint32_t destroySocket(pal_socket_t sfd)
{
  pal_shutdown( sfd, PAL_SOCK_SHTDN_BOTH );
  pal_closesocket( sfd );

  return 0;
}

/* Thread routine to calculate the distance for a given broker */
pal_thread_ret_t PAL_THREAD_CALL tspGetBrokerDistance(void *threadarg)
{
  tBrokerTimingThreadArg *arguments = NULL;
  tBrokerList *broker = NULL;
  tConf *conf = NULL;
  tRedirectStatus status = TSP_REDIRECT_OK;
  unsigned int distance = 0;

  /* The thread needs a broker argument to work with */
  if (threadarg == NULL) {
    pal_thread_exit((pal_thread_ret_t)(-1));
    return (pal_thread_ret_t)(-1);
  }

  /* Unwrap the arguments */
  arguments = (tBrokerTimingThreadArg *)threadarg;
  broker = (tBrokerList *)arguments->broker;
  conf = (tConf *)arguments->conf;

  /* Perform the echo request, calculating the distance */
  status = tspDoEchoRequest(broker->address, broker->address_type, conf, &distance);

  /* Set the calculated distance in the broker list element */
  broker->distance = distance;

  /* Make the status available on join */
  pal_thread_exit((pal_thread_ret_t)status);
  return (pal_thread_ret_t)status;
}

/* Fill the distance values for the brokers in a list */
tRedirectStatus tspGetBrokerDistances(tBrokerList *broker_list, int broker_count, tConf *conf)
{
  sint32_t rc = 0;
  int t = 0;
  tRedirectStatus status = TSP_REDIRECT_OK;
  pal_thread_ret_t thread_status = 0;
  pal_thread_t *threads = NULL;
  tBrokerTimingThreadArg *thread_arguments = NULL;
  tBrokerList *broker_list_index = NULL;

  /* Initialize thread array */
  if ((threads = (pal_thread_t *)malloc(broker_count * sizeof(pal_thread_t))) == NULL) {
    Display(LOG_LEVEL_1, ELError, "tspGetBrokerDistances", GOGO_STR_RDR_CANT_MALLOC_THREAD_ARRAY);
    return TSP_REDIRECT_CANT_MALLOC_THREAD_ARRAY;
  }

  /* Initialize thread argument array */
  if ((thread_arguments = (tBrokerTimingThreadArg *)malloc(broker_count * sizeof(tBrokerTimingThreadArg))) == NULL) {
    free(threads);
    Display(LOG_LEVEL_1, ELError, "tspGetBrokerDistances", GOGO_STR_RDR_CANT_MALLOC_THREAD_ARGS);
    return TSP_REDIRECT_CANT_MALLOC_THREAD_ARGS;
  }

  /* We start at the beginning of the broker list */
  broker_list_index = broker_list;

  /* Loop through the broker list */
  for (t = 0; ((t < broker_count) && (broker_list_index != NULL)); t++)
  {
    Display(LOG_LEVEL_3, ELInfo, "tspGetBrokerDistances", GOGO_STR_RDR_CREATING_DISTANCE_THREAD, broker_list_index->address);

    /* Set the thread arguments */
    thread_arguments[t].broker = broker_list_index;
    thread_arguments[t].conf = conf;

    /* Start the distance calculation thread for that broker in the list */
    rc = pal_thread_create( &threads[t], &tspGetBrokerDistance, (void *)&thread_arguments[t] );

    /* If we can't create the thread, return an error */
    if( rc != 0 )
    {
      free(threads);
      free(thread_arguments);
      Display(LOG_LEVEL_1, ELError, "tspGetBrokerDistances", GOGO_STR_RDR_CANT_CREATE_DISTANCE_THREAD, broker_list_index->address);
      return TSP_REDIRECT_CANT_CREATE_THREAD;
    }

    /* Move to the next broker in the list */
    broker_list_index = broker_list_index->next;
  }

  /* We start from the begining of the list again to join the threads */
  broker_list_index = broker_list;

  /* Loop through the broker list */
  for (t = 0; ((t < broker_count) && (broker_list_index != NULL)); t++)
  {
    Display(LOG_LEVEL_3, ELInfo, "tspGetBrokerDistances", GOGO_STR_RDR_WAITING_FOR_THREAD, broker_list_index->address);

    /* Try to join the thread corresponding to the broker in the list */
    rc = pal_thread_join( threads[t], (pal_thread_ret_t*)&thread_status );

    /* If we can't join the thread, return an error */
    if( rc != 0 )
    {
      free(threads);
      free(thread_arguments);
      Display(LOG_LEVEL_1, ELError, "tspGetBrokerDistances", GOGO_STR_RDR_ERR_WAITING_FOR_THREAD, broker_list_index->address);
      return TSP_REDIRECT_CANT_WAIT_FOR_THREAD;
    }

    status = (tRedirectStatus)thread_status;

    /* The distance was calculated correctly */
    if (status == TSP_REDIRECT_OK) {
      Display(LOG_LEVEL_3, ELInfo, "tspGetBrokerDistances", GOGO_STR_RDR_DISTANCE_CALCULATION_OK, broker_list_index->address, broker_list_index->distance);
    }
    /* Echo requests timed out */
    else if (status == TSP_REDIRECT_ECHO_REQUEST_TIMEOUT) {
      Display(LOG_LEVEL_3, ELInfo, "tspGetBrokerDistances", GOGO_STR_RDR_DISTANCE_CALCULATION_TIMEOUT, broker_list_index->address);
    }
    /* There was an error somewhere */
    else {
      Display(LOG_LEVEL_1, ELError, "tspGetBrokerDistances", GOGO_STR_RDR_DISTANCE_CALCULATION_ERR, broker_list_index->address);
    }

    /* We move to the next broker in the list */
    broker_list_index = broker_list_index->next;
  }

  free(threads);
  free(thread_arguments);

  return TSP_REDIRECT_OK;
}

/* Calculate the roundtrip time to a brokre using a TSP echo request */
tRedirectStatus timeEchoRequestReply(pal_socket_t sfd, char *address, rttengine_stat_t *engine, uint32_t *distance) {
  char data_in[ECHO_REQUEST_IN_BUF_SIZE];
  char data_out[ECHO_REQUEST_OUT_BUF_SIZE];
  sint32_t data_in_size = 0;
  size_t data_out_size = 0;
  rudp_msghdr_t *imh = NULL;
  rudp_msghdr_t *omh = NULL;
  void *om = NULL;
  void *im = NULL;
  sint32_t length_sent = 0;
  sint32_t ret = 0;
  fd_set fs;
  struct timeval tv_select;

  /* The receive buffer is empty */
  memset(data_in, 0, sizeof(data_in));
  /* The send buffer contains the "ECHO REQUEST" command */
  pal_snprintf(data_out, sizeof(data_out), ECHO_REQUEST_COMMAND);

  /* Calculate the buffer sizes */
  data_in_size = sizeof(data_in);
  data_out_size = pal_strlen(data_out);

  /* Prepare RUDP messages */
  im = internal_prepare_message(&imh, data_in_size);
  om = internal_prepare_message(&omh, data_out_size);

  /* A NULL message is an error */
  if ((im == NULL) || (om == NULL)) {
    rttengine_deinit(engine, im, om);
    *distance += ECHO_REQUEST_ERROR_ADJUST;
    Display(LOG_LEVEL_1, ELError, "timeEchoRequestReply", GOGO_STR_RDR_ERROR_PREPARING_RUDP_MSG, address);
    return TSP_REDIRECT_ECHO_REQUEST_ERROR;
  }

  /* Zero them out */
  memset(im, 0, data_in_size);
  memset(om, 0, data_out_size);

  /* Copy the outgoing data to the data part of the outgoing message */
  memcpy((char*)om + sizeof(rudp_msghdr_t), data_out, data_out_size);

  /* Set the outgoing message's sequence number */
  omh->sequence = htonl(engine->sequence++ | 0xf0000000);

send_loop:

  /* Fail if we have reached the maximum number of echo request attempts */
  if (engine->retries == ECHO_REQUEST_ATTEMPTS) {
    rttengine_deinit(engine, im, om);
    *distance += ECHO_REQUEST_TIMEOUT_ADJUST;
    Display(LOG_LEVEL_3, ELWarning, "timeEchoRequestReply", GOGO_STR_RDR_MAX_ECHO_REPLY_ATTEMPTS, ECHO_REQUEST_ATTEMPTS, address);
    return TSP_REDIRECT_ECHO_REQUEST_TIMEOUT;
  }

  /* Set the timestamp for the outgoing message */
  omh->timestamp = htonl(internal_get_timestamp(engine));

  /* Try to send the outgoing message */
  Display(LOG_LEVEL_3, ELInfo, "timeEchoRequestReply", GOGO_STR_RDR_SENDING_ECHO_REQUEST, (engine->retries + 1), address);

  if ((length_sent = send(sfd, om, (sint32_t)(data_out_size + sizeof(rudp_msghdr_t)), 0)) == -1) {
    rttengine_deinit(engine, im, om);
    *distance += ECHO_REQUEST_ERROR_ADJUST;
    Display(LOG_LEVEL_1, ELError, "timeEchoRequestReply", GOGO_STR_RDR_SEND_ECHO_REQUEST_FAILED, address);
    return TSP_REDIRECT_ECHO_REQUEST_ERROR;
  }

  /* Set the timeout for the select */
  /* This is the maximum time we will wait for an echo reply */
  tv_select.tv_sec = ECHO_REQUEST_TIMEOUT / 1000;
  tv_select.tv_usec = (ECHO_REQUEST_TIMEOUT % 1000) * 1000;

select_loop:

  FD_ZERO(&fs);
  FD_SET(sfd, &fs);

  Display(LOG_LEVEL_3, ELInfo, "timeEchoRequestReply", GOGO_STR_RDR_WAITING_ECHO_REPLY, address);

  /* Wait on the socket set */
  ret = select((sint32_t)sfd + 1, &fs, NULL, NULL, &tv_select);

  switch(ret)
  {
    /* Select timed out, try again */
    case 0:
      Display(LOG_LEVEL_3, ELWarning, "timeEchoRequestReply", GOGO_STR_RDR_WAITING_ECHO_REPLY_TIMEOUT, address);
      engine->retries++;
      goto send_loop;
    /* Ok, select got something */
    case 1:
      Display(LOG_LEVEL_3, ELWarning, "timeEchoRequestReply", GOGO_STR_RDR_RECEIVING_RUDP_MESSAGE, address);

      /* Receive the incoming data, and store it in the incoming message */
      ret = recv(sfd, im, (sizeof(rudp_msghdr_t) + data_in_size), 0);

      /* This is a fatal read error */
      if (ret == -1) {
        rttengine_deinit(engine, im, om);
        *distance += ECHO_REQUEST_ERROR_ADJUST;
        Display(LOG_LEVEL_1, ELError, "timeEchoRequestReply", GOGO_STR_RDR_ERR_RECEIVING_RUDP_FROM, address);
        return TSP_REDIRECT_ECHO_REQUEST_ERROR;
      }

      /* If we have the same sequence number, this is the message we want */
      if (imh->sequence == omh->sequence) {
        *distance += internal_get_timestamp(engine) - ntohl(omh->timestamp);
        ret = ret - sizeof(rudp_msghdr_t);
        Display(LOG_LEVEL_3, ELInfo, "timeEchoRequestReply", GOGO_STR_RDR_RECEIVED_RUDP_OK, address);
        break;
      }
      /* If the sequence numbers are different, see if we get something else */
      else {
        Display(LOG_LEVEL_3, ELWarning, "timeEchoRequestReply", GOGO_STR_RDR_RECV_RUDP_SEQ_DIFFERS, address);

        goto select_loop;
      }
    /* This is an unknown error */
    default:
      rttengine_deinit(engine, im, om);
      *distance += ECHO_REQUEST_ERROR_ADJUST;
      Display(LOG_LEVEL_1, ELError, "timeEchoRequestReply", GOGO_STR_RDR_ERR_WAITING_ECHO_REPLY, address);
      return TSP_REDIRECT_ECHO_REQUEST_ERROR;
  }

  /* Update the stat engine */
  rttengine_update(engine, (internal_get_timestamp(engine) - ntohl(imh->timestamp)));

  /* Copy the data part of the incoming message to the receiving buffer */
  memcpy(data_in, (char*)im + sizeof(rudp_msghdr_t), ret);

  /* Free the messages */
  internal_discard_message(im);
  internal_discard_message(om);

  engine->retries = 0;

  /* Validate that we got the right answer from the broker */
  if (tspGetStatusCode(data_in) != ECHO_REQUEST_SUCCESS_STATUS) {
    Display(LOG_LEVEL_1, ELError, "timeEchoRequestReply", GOGO_STR_RDR_RCV_UNEXPECTED_ECHO_REPLY, address, data_in);
    return TSP_REDIRECT_ECHO_REQUEST_ERROR;
  }

  Display(LOG_LEVEL_3, ELInfo, "timeEchoRequestReply", GOGO_STR_RDR_RCV_EXPECTED_ECHO_REPLY, address, data_in);

  return TSP_REDIRECT_OK;
}

/* Get a socket address structure from a server string */
tSocketAddressStatus getSocketAddress(char *server, tBrokerAddressType address_type, tTunnelMode tunnel_mode, struct addrinfo **address_info_root, struct addrinfo **address_info) {
  struct addrinfo hints;
  struct addrinfo *result = NULL;
  tSocketAddressStatus status = SOCKET_ADDRESS_OK;
  struct addrinfo *result_index = NULL;
  sint32_t need_v6_endpoint = 0;
  sint32_t found_family_address = 0;

  /* Zero out the hints */
  memset(&hints, 0, sizeof(struct addrinfo));

  /* We're doing UDP */
  hints.ai_socktype = SOCK_DGRAM;

  /* Determine if we need to connect to an IPv6 address */
  /* based on the configured tunnel mode */
  if (tunnel_mode == V4V6) {
    need_v6_endpoint = 1;
  }

  /* Set some more hints based on the type of address */
  /* present in the broker list */
  switch (address_type) {
    case TSP_REDIRECT_BROKER_TYPE_NONE:
      return SOCKET_ADDRESS_ERROR;
    case TSP_REDIRECT_BROKER_TYPE_IPV4:
      hints.ai_family = AF_INET;
      hints.ai_flags |= AI_NUMERICHOST;
      break;
    case TSP_REDIRECT_BROKER_TYPE_IPV6:
      hints.ai_family = AF_INET6;
      hints.ai_flags |= AI_NUMERICHOST;
      break;
    case TSP_REDIRECT_BROKER_TYPE_FQDN:
      hints.ai_family = AF_UNSPEC;
      break;
  }

  /* Get an address structure using the broker address and the hints */
  if ((getaddrinfo(server, SERVER_PORT_STR, &hints, &result)) != 0) {
    if (result != NULL) {
      freeaddrinfo(result);
    }

    return SOCKET_ADDRESS_PROBLEM_RESOLVING;
  }

  /* We start with the first address found */
  result_index = result;

  switch (address_type) {
    case TSP_REDIRECT_BROKER_TYPE_NONE:
      return SOCKET_ADDRESS_ERROR;
    case TSP_REDIRECT_BROKER_TYPE_IPV4:
      if (need_v6_endpoint) {
        /* We need IPv6, but it's an IPv4 */
        status = SOCKET_ADDRESS_WRONG_FAMILY;
      }
      else {
        status = SOCKET_ADDRESS_OK;
      }
      break;
    case TSP_REDIRECT_BROKER_TYPE_IPV6:
      if (need_v6_endpoint) {
        status = SOCKET_ADDRESS_OK;
      }
      else {
        /* We need IPv4, but it's an IPv6 */
        status = SOCKET_ADDRESS_WRONG_FAMILY;
      }
      break;
    case TSP_REDIRECT_BROKER_TYPE_FQDN:
      /* It's an FQDN, try to find an address of the correct family */
      for (result_index = result; result_index; result_index = result_index->ai_next) {
        if  ((need_v6_endpoint && (result_index->ai_family == AF_INET6)) ||
          (!need_v6_endpoint && (result_index->ai_family == AF_INET))) {

          /* We found one */
          found_family_address = 1;
          break;
        }
      }

      /* If we found an address of the right family, all is ok */
      if (found_family_address) {
        status = SOCKET_ADDRESS_OK;
      }
      /* If we did not, there's a family problem and we take the */
      /* first address of the wrong family */
      else {
        status = SOCKET_ADDRESS_WRONG_FAMILY;
        result_index = result;
      }

      break;
  }

  /* Set the address info root pointer so we can free it later */
  *address_info_root = result;
  /* Set the pointer to the address information */
  *address_info = result_index;

  return status;
}

/* Initialize what's needed, perform calculation using an echo */
/* request, and deinitialize */
tRedirectStatus tspDoEchoRequest(char *address, tBrokerAddressType address_type, tConf *conf, uint32_t *distance) {
  rttengine_stat_t *engine = NULL;
  struct addrinfo *address_info = NULL;
  struct addrinfo *address_info_root = NULL;
  pal_socket_t sfd;
  tSocketAddressStatus socket_address_status = SOCKET_ADDRESS_OK;
  tRedirectStatus status = TSP_REDIRECT_OK;

  /* Initilalize some stuff */
  *distance = 0;

  /* Get an address structure and see if we have the right address family */
  socket_address_status = getSocketAddress(address, address_type, conf->tunnel_mode, &address_info_root, &address_info);

  /* Wrong family, log and modify the distance so it goes at the end of the sorted list */
  if (socket_address_status == SOCKET_ADDRESS_WRONG_FAMILY) {
    *distance = ECHO_REQUEST_WRONG_FAMILY_ADJUST;
    Display(LOG_LEVEL_1, ELError, "tspDoEchoRequest", GOGO_STR_RDR_WRONG_ADDRESS_FAMILY, address);
  }
  /* There was some kind of error, adjust the distance to make it go after the brokers that */
  /* could be timed properly. */
  else if (socket_address_status == SOCKET_ADDRESS_ERROR) {
    *distance = ECHO_REQUEST_ERROR_ADJUST;
    if (address_info_root != NULL) {
      freeaddrinfo(address_info_root);
    }
    Display(LOG_LEVEL_1, ELError, "tspDoEchoRequest", GOGO_STR_RDR_ERROR_GET_SOCKADDRESS, address);
    return TSP_REDIRECT_ECHO_REQUEST_ERROR;
  }
  /* Could not resolve, this is treated the same as an error */
  else if (socket_address_status == SOCKET_ADDRESS_PROBLEM_RESOLVING) {
    *distance = ECHO_REQUEST_ERROR_ADJUST;
    if (address_info_root != NULL) {
      freeaddrinfo(address_info_root);
    }
    Display(LOG_LEVEL_1, ELError, "tspDoEchoRequest", GOGO_STR_RDR_ERROR_RESOLVING_DN, address);
    return TSP_REDIRECT_ECHO_REQUEST_ERROR;
  }

  /* Connect to the address */
  if ((sfd = createConnectedSocket(address_info)) == -1) {
    *distance += ECHO_REQUEST_ERROR_ADJUST;
    if (address_info_root != NULL) {
      freeaddrinfo(address_info_root);
    }
    destroySocket(sfd);
    Display(LOG_LEVEL_1, ELError, "tspDoEchoRequest", GOGO_STR_RDR_ERROR_CONNECT_SOCKET, address);
    return TSP_REDIRECT_ECHO_REQUEST_ERROR;
  }

  if (address_info_root != NULL) {
    freeaddrinfo(address_info_root);
  }

  /* Create a stat engine */
  if ((engine = createStatEngine()) == NULL) {
    *distance += ECHO_REQUEST_ERROR_ADJUST;
    destroySocket(sfd);
    Display(LOG_LEVEL_1, ELError, "tspDoEchoRequest", GOGO_STR_RDR_ERROR_CREATE_STAT_ENGINE, address);
    return TSP_REDIRECT_ECHO_REQUEST_ERROR;
  }

  /* Initialize the stat engine */
  if (rttengine_init(engine) != 1) {
    *distance += ECHO_REQUEST_ERROR_ADJUST;
    destroySocket(sfd);
    pal_free(engine);
    Display(LOG_LEVEL_1, ELError, "tspDoEchoRequest", GOGO_STR_RDR_ERROR_INIT_STAT_ENGINE, address);
    return TSP_REDIRECT_ECHO_REQUEST_ERROR;
  }

  /* Calculate the roundtrip time */
  status = timeEchoRequestReply(sfd, address, engine, distance);

  /* Destroy the socket */
  destroySocket(sfd);

  /* Uninitialize the stat engine */
  rttengine_deinit(engine, NULL, NULL);

  /* Free the stat engine */
  pal_free(engine);

  return status;
}

