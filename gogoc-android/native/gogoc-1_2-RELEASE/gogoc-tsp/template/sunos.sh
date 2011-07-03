#!/bin/sh
#
# $Id: sunos.sh,v 1.3 2010/03/07 19:31:18 carl Exp $
#
# This source code copyright (c) gogo6 Inc. 2002-2005.
#
# For license information refer to CLIENT-LICENSE.TXT
#

LANGUAGE=C

if [ -z "$TSP_VERBOSE" ]; then
   TSP_VERBOSE=0
fi

KillProcess()
{
   if [ ! -z $TSP_VERBOSE ]; then
      if [ $TSP_VERBOSE -ge 2 ]; then
         echo killing $*
      fi
   fi
   pkill `basename $1`
}

Display()
{
   if [ -z "$TSP_VERBOSE" ]; then
      return;
   fi
   if [ $TSP_VERBOSE -lt $1 ]; then
      return;
   fi
   shift
   echo $*
}

Exec()
{
   if [ ! -z "$TSP_VERBOSE" ]; then
      if [ $TSP_VERBOSE -ge 2 ]; then
         echo $*
      fi
   fi
   $*
   ret=$?
   if [ $ret -ne 0 ]; then
      echo "Error while executing $1"
      echo "   Command: $*"
      exit 1
   fi
   echo "Command $1 succeed"
}

# Program localization 

Display 1 "--- Start of configuration script. ---"
Display 1 "Script: " `basename $0`

ifconfig=/sbin/ifconfig
route=/usr/sbin/route
ndpd=/usr/lib/inet/in.ndpd
ndd=/usr/sbin/ndd
resolv_conf=/etc/resolv.conf

ndpdconfigfilename=ndpd.conf
ndpdconfigfile=$TSP_HOME_DIR/$ndpdconfigfilename

if [ -z "$TSP_HOME_DIR" ]; then
   echo "TSP_HOME_DIR variable not specified!;"
   exit 1
fi

if [ ! -d $TSP_HOME_DIR ]; then
   echo "Error : directory $TSP_HOME_DIR does not exist"
   exit 1
fi
#

if [ -z "$TSP_HOST_TYPE" ]; then
   echo Error: TSP_HOST_TYPE not defined.
   exit 1
fi


#################################
# Run tunnel destruction script.
#################################
if [ X"${TSP_OPERATION}" = X"TSP_TUNNEL_TEARDOWN" ]; then

  Display 1 "Tunnel tear down starting..."

  #
  # DNS
  Display 1 "Removing DNS server"
  Display 1 "NOTE: Adjust template script to perform actions"
  # rm ${resolv_conf}
  # if [ -f ${resolv_conf}.bak ]; then
  #   cp ${resolv_conf}.bak ${resolv_conf}
  #   rm ${resolv_conf}.bak
  # fi

  # Disable neighbor discovery.
  if [ X"${TSP_HOST_TYPE}" = X"router" ]; then

    if [ -f "$ndpd" ]; then
      KillProcess $ndpd
    fi

    # Remove PREFIX::1 address from HOME interface.
    $ifconfig $TSP_HOME_INTERFACE inet6 removeif $TSP_PREFIX::1

    $ndd -set /dev/ip ip6_forwarding 0
    $ndd -set /dev/ip ip6_send_redirects 0
    $ndd -set /dev/ip ip6_ignore_redirect 0
  fi

  # Remove INET6 default route
  $route delete -inet6 default $TSP_SERVER_ADDRESS_IPV6 2>/dev/null

  # Unplumb and disable tunnel interface
  $ifconfig $TSP_TUNNEL_INTERFACE inet6 unplumb down 2>/dev/null

  Display 1 "Tunnel tear down completed."

  exit 0
fi


##########################
# Tunnel creation script.
##########################
if [ X"${TSP_HOST_TYPE}" = X"host" ] || [ X"${TSP_HOST_TYPE}" = X"router" ]; then

  Display 1 "Tunnel set up starting..."

  # Unplumb the tunnel interface and bring it down.
  $ifconfig $TSP_TUNNEL_INTERFACE inet6 unplumb down 2>/dev/null
   
  # Create tunnel on physical interface. At this point the physical 
  # interface will have IPv6 link-local addresses.
  Exec $ifconfig $TSP_TUNNEL_INTERFACE inet6 plumb tsrc $TSP_CLIENT_ADDRESS_IPV4 tdst $TSP_SERVER_ADDRESS_IPV4 up

  # Configure IPv6 addresses tunnel endpoints, by creating new logical 
  # interface (:1). This is required because we're using global IPv6 
  # addresses (compared to the link-local ones on the physical).
  Exec $ifconfig $TSP_TUNNEL_INTERFACE inet6 addif $TSP_CLIENT_ADDRESS_IPV6 $TSP_SERVER_ADDRESS_IPV6 up

  # Set physical interface MTU to 1280.
  Exec $ifconfig $TSP_TUNNEL_INTERFACE inet6 mtu 1280
  # Set logical interface MTU to 1280.
  Exec $ifconfig ${TSP_TUNNEL_INTERFACE}:1 inet6 mtu 1280

  # Default route: remove route and add new
  $route delete -inet6 default $TSP_SERVER_ADDRESS_IPV6 2>/dev/null
  Exec $route add -inet6 default $TSP_SERVER_ADDRESS_IPV6
  #
  # DNS
  if [ X"${TSP_CLIENT_DNS_ADDRESS_IPV6}" != X"" ]; then
    Display 1 "Adding DNS server"
    Display 1 "NOTE: Adjust template script to perform actions"
    # cp ${resolv_conf} ${resolv_conf}.bak
    # echo "nameserver ${TSP_CLIENT_DNS_ADDRESS_IPV6}" | cat - ${resolv_conf}.bak >${resolv_conf}
  fi
fi

# Host only configuration
if [ X"${TSP_HOST_TYPE}" = X"host" ]; then

   Exec $ndd -set /dev/ip ip6_forwarding 0
   Exec $ndd -set /dev/ip ip6_send_redirects 0
   Exec $ndd -set /dev/ip ip6_ignore_redirect 0
fi

# Router configuration if required
if [ X"${TSP_HOST_TYPE}" = X"router" ]; then

  Display 1 "Router configuration"

  # Kernel setup.
  Exec $ndd -set /dev/ip ip6_forwarding 1
  Exec $ndd -set /dev/ip ip6_send_redirects 1
  Exec $ndd -set /dev/ip ip6_ignore_redirect 1

  # Put the PREFIX::1 address on HOME interface(Remove it if it was there).
  $ifconfig $TSP_HOME_INTERFACE inet6 removeif $TSP_PREFIX::1
  Exec $ifconfig $TSP_HOME_INTERFACE inet6 addif $TSP_PREFIX::1/64 up 

  # Check if router advertisement daemon exists.
  if [ -f "$ndpd" ]; then

    KillProcess $ndpd

    # Create configuration file.
    cat > "$ndpdconfigfile" <<EOF
##### $ndpdconfigfilename made by gogoCLIENT #####
ifdefault AdvSendAdvertisements true
prefix $TSP_PREFIX::/64 $TSP_HOME_INTERFACE
EOF

    # Launch router advertisement daemon.
    Exec $ndpd -f $ndpdconfigfile
  fi
fi

Display 1 "Tunnel set up finished"

exit 0
