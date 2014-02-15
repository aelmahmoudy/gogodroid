#!/bin/sh

# Copyright (c) 2001-2003,2006,2007 gogo6 Inc. All rights reserved.
#
# For license information refer to CLIENT-LICENSE.TXT
#
# ****************************************
# * Tunnel Server Protocol version 1.0   *
# * Host configuration script            *
# * For Cisco Routers                    *
# * Lanched from a unix OS               *
# ****************************************
# * This script keeps a log              *
# ****************************************

CISCO="$TSP_HOME_DIR/ciscoipv6.txt"

if [ -z $TSP_VERBOSE ]; then
	TSP_VERBOSE=0
fi

if [ $TSP_VERBOSE -ge 1 ]; then
	LOG="/dev/stdout"
else
	LOG="/dev/null" 
fi

# ***** Script an IPv6 tunnel **********

echo "IPv4 tunnel server address configured : $TSP_SERVER_ADDRESS_IPV4" > $LOG
echo "! Add these lines to your Cisco configuration" > $CISCO
echo "! Script launched from a `uname` environment" >> $CISCO
echo "! Generated `date`" >> $CISCO
echo "!" >> $CISCO
echo "ipv6 unicast-routing" >> $CISCO
echo "!" >> $CISCO
echo "interface $TSP_TUNNEL_INTERFACE" >> $CISCO
echo " ipv6 address $TSP_CLIENT_ADDRESS_IPV6/128" >> $CISCO
echo " tunnel source $TSP_CLIENT_ADDRESS_IPV4" >> $CISCO
echo " tunnel destination $TSP_SERVER_ADDRESS_IPV4" >> $CISCO
echo " tunnel mode ipv6ip" >> $CISCO
echo "!" >> $CISCO
echo "ipv6 route ::/0 $TSP_TUNNEL_INTERFACE" >> $CISCO

if [ X"$TSP_HOST_TYPE" = X"router" ]; then 
echo "cisco Router will be configured as IPv6 router and it will do router advertisements for autoconfiguration" > $LOG
echo "Configuring IPv6_forwarding on network interface" > $LOG
echo "!" >> $CISCO
echo "interface $TSP_HOME_INTERFACE" >> $CISCO
echo " ipv6 address $TSP_PREFIX:1::/64 eui-64" >> $CISCO
echo " ipv6 nd prefix-advertisement $TSP_PREFIX:1::/64 43200 43200 onlink autoconfig" >> $CISCO
fi

echo "Cisco IPv6 configuration generated and saved in this file : $CISCO" > $LOG
echo "Look at the file and apply these configuration lines to your Cisco configuration" > $LOG

echo "End of the script" > $LOG

