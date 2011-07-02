// **************************************************************************
// $Id: gogocmsgdata.h,v 1.2 2010/03/07 16:30:44 carl Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
//
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   This include file describes the domain values and data that will be
//   exchanged between the gogoCLIENT GUI and gogoCLIENT.
//
//   These structures must be defined in C-style for integration
//   in the gogoCLIENT.
//
//   You may extend the structures to include new data, - BUT -
//   Remember to do the following:
//   - Translate the new data.
//   - Encode then new data.
//   - *THINK* of backwards compatibility (Maybe crate a new message ID
//      for the extended structure and preserve functionnality).
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_gogocmsgdata_h__
#define __gogocmessaging_gogocmsgdata_h__


#include <time.h>


// gogoCLIENT status information: gogocCliStatus - (Enumeration)
//   - DISCONNECTEDIDLE: Client is disconnected (idle: No connection attempt)
//   - DISCONNECTEDERROR: Client is disconnected because of an error.
//   - DISCONNECTEDNORETRY: Client was connected and will NOT retry.
//   - DISCONNECTEDHACCESSSETUPERROR: Client was disconnected because of an HACCESS setup error.
//   - DISCONNECTEDHACCESSEXPOSEDEVICESERROR: Client was disconnected because of an HACCESS device mapping error.
//   - CONNECTING: Client is negotiating tunnel and setting up.
//   - CONNECTED: Client is successfully connected.
//
typedef enum { GOGOC_CLISTAT__DISCONNECTEDIDLE,
               GOGOC_CLISTAT__DISCONNECTEDNORETRY,
               GOGOC_CLISTAT__DISCONNECTEDERROR,
               GOGOC_CLISTAT__DISCONNECTEDHACCESSSETUPERROR,
               GOGOC_CLISTAT__DISCONNECTEDHACCESSEXPOSEDEVICESERROR,
               GOGOC_CLISTAT__CONNECTING,
               GOGOC_CLISTAT__CONNECTED } gogocCliStatus;


// gogoCLIENT tunnel types: gogocTunnelType - (Enumeration)
//   (See application manual if you don't understand these values).
typedef enum { TUNTYPE_V6V4,
               TUNTYPE_V6UDPV4,
               TUNTYPE_V4V6 } gogocTunnelType;


// gogoCLIENT status information: gogocStatusInfo - (Data structure)
//   - eStatus: The status, as described earlier.
//   - szStatus: Additionnal information on the status (error, mostly).
//
typedef struct __STATUS_INFO
{
  gogocCliStatus eStatus;
  signed int    nStatus;
} gogocStatusInfo;


// gogoCLIENT tunnel information: gogocTunnelInfo - (Data structure)
//   - szBrokerName: The name of the broker used for tunnel negotiation.
//   - eTunnelType: Type of tunnel.
//   - szIPV4AddrLocalEndpoint: Local tunnel endpoint IPv4 address.
//   - szIPV6AddrLocalEndpoint: Local tunnel endpoint IPv6 address.
//   - szIPV6AddrDns: DNS IPv6 address.
//   - szIPV4AddrRemoteEndpoint: Remote tunnel endpoint IPv4 address.
//   - szIPV6AddrRemoteEndpoint: Remote tunnel endpoint IPv6 address.
//   - szDelegatedPrefix: The delegated prefix (if routing is enabled).
//   - szUserDomain: The domain delegated to the used for his prefix.
//   - tunnelUpTime: c-time at which the tunnel was 'up'. NOT THE UPTIME.
//
typedef struct __TUNNEL_INFO
{
  char* szBrokerName;
  gogocTunnelType eTunnelType;
  char* szIPV4AddrLocalEndpoint;
  char* szIPV6AddrLocalEndpoint;
  char* szIPV6AddrDns;
  char* szIPV4AddrRemoteEndpoint;
  char* szIPV6AddrRemoteEndpoint;
  char* szDelegatedPrefix;
  char* szUserDomain;
  time_t tunnelUpTime;
} gogocTunnelInfo;


// gogoCLIENT broker list: gogocBrokerList - (Data structure)
//   - szBrokerName: Name of the broker.
//   - next: Next element in the list: NULL if end.
//
struct __BROKER_LIST;
typedef struct __BROKER_LIST
{
  char* szAddress;
  int   nDistance;
  struct __BROKER_LIST* next;
} gogocBrokerList;


#endif
