#!/bin/sh
#$Id: freebsd.sh,v 1.3 2010/03/07 19:31:17 carl Exp $
#
# This source code copyright (c) gogo6 Inc. 2002-2004,2007.
#
# This program is free software; you can redistribute it and/or modify it 
# under the terms of the GNU General Public License (GPL) Version 2, 
# June 1991 as published by the Free  Software Foundation.
#
# For license information refer to CLIENT-LICENSE.TXT
#

LANGUAGE=C

if [ -z $TSP_VERBOSE ]; then
   TSP_VERBOSE=0
fi

KillProcess()
{
   if [ ! -z $TSP_VERBOSE ]; then
      if [ $TSP_VERBOSE -ge 2 ]; then
         echo killing $*
      fi
   fi
   PID=`ps axww | grep $1 | grep -v grep | awk '{ print $1;}'`
   echo $PID
   if [ ! -z $PID ]; then
      kill $PID
   fi
}

Display()
{
   if [ -z $TSP_VERBOSE ]; then
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
   if [ ! -z $TSP_VERBOSE ]; then
      if [ $TSP_VERBOSE -ge 2 ]; then
         echo $*
      fi
   fi
   $* # Execute command
   if [ $? -ne 0 ]; then
      echo "Error while executing $1"
      echo "   Command: $*"
      exit 1
   fi
}

ExecNoCheck()
{
   if [ ! -z $TSP_VERBOSE ]; then
      if [ $TSP_VERBOSE -ge 2 ]; then
         echo $*
      fi
   fi
   $* # Execute command
}

# Program localization 

Display 1 '--- Start of configuration script. ---'
Display 1 "Script: " `basename $0`

gifconfig=/usr/sbin/gifconfig
ifconfig=/sbin/ifconfig
route=/sbin/route
rtadvd=/usr/sbin/rtadvd
sysctl=/sbin/sysctl
# find the release rel=4 for freebsd4.X, =5 for freebsd5.X
rel=`/usr/bin/uname -r | cut -f1 -d"."`
resolv_conf=/etc/resolv.conf

if [ -z $TSP_HOME_DIR ]; then
   echo "TSP_HOME_DIR variable not specified!;"
   exit 1
fi

if [ ! -d $TSP_HOME_DIR ]; then
   echo "Error : directory $TSP_HOME_DIR does not exist"
   exit 1
fi
#

if [ -z $TSP_HOST_TYPE ]; then
   echo Error: TSP_HOST_TYPE not defined.
   exit 1
fi


#################################
# Run tunnel destruction script.
#################################
if [ X"${TSP_OPERATION}" = X"TSP_TUNNEL_TEARDOWN" ]; then

  Display 1 Tunnel tear down starting...

  #
  # DNS
  Display 1 "Removing DNS server"
  Display 1 "NOTE: Adjust template script to perform actions"
  # ExecNoCheck rm ${resolv_conf}
  # if [ -f ${resolv_conf}.bak ]; then
  #   ExecNoCheck cp ${resolv_conf}.bak ${resolv_conf}
  #   ExecNoCheck rm ${resolv_conf}.bak
  # fi

  # Router deconfiguration.
  if [ X"${TSP_HOST_TYPE}" = X"router" ]; then
    # Remove prefix routing on TSP_HOME_INTERFACE
    ExecNoCheck $route delete -inet6 $TSP_PREFIX::

    # Remove blackhole.
    if [ X"${TSP_PREFIXLEN}" != X"64" ]; then
      ExecNoCheck $route delete -inet6 $TSP_PREFIX:: -prefixlen $TSP_PREFIXLEN -interface lo0
    fi

    # Remove static IPv6 address
    ExecNoCheck $ifconfig $TSP_HOME_INTERFACE inet6 $TSP_PREFIX::1 delete

    # Kill router advertisement daemon gracefully (SIGTERM)
    KillProcess rtadvd
  fi

  # Delete any routes
  if [ X"${TSP_TUNNEL_MODE}" = X"v6v4" ] || [ X"${TSP_TUNNEL_MODE}" = X"v6udpv4" ]; then
    # Delete any default IPv6 route
    ExecNoCheck $route delete -inet6 default
  else 
    # Delete default IPv4 route
    ExecNoCheck $route delete -inet default
  fi


  # Check if interface exists and remove it
  $ifconfig $TSP_TUNNEL_INTERFACE >/dev/null 2>/dev/null
  if [ $? -eq 0 ]; then

    if [ X"${TSP_TUNNEL_MODE}" = X"v6v4" ] || [ X"${TSP_TUNNEL_MODE}" = X"v6udpv4" ]; then

      # Delete interface IPv6 configuration.
      list=`$ifconfig $TSP_TUNNEL_INTERFACE | grep inet6 | awk '{print $2}' | grep -v '^fe80'`
      for ipv6address in $list
      do 
        ExecNoCheck $ifconfig $TSP_TUNNEL_INTERFACE inet6 $ipv6address delete
      done
    else
      # Delete interface IPv4 configuration.
      list=`$ifconfig $TSP_TUNNEL_INTERFACE | grep -v inet6 | grep inet | awk '{print $2}'`
      for ipv4address in $list
      do 
        ExecNoCheck $ifconfig $TSP_TUNNEL_INTERFACE inet $ipv4address delete
      done
    fi    

    # Bring interface down and TRY to destroy it (FreeBSD 6.2 STABLE and up).
    ExecNoCheck $ifconfig $TSP_TUNNEL_INTERFACE down
    ExecNoCheck $ifconfig $TSP_TUNNEL_INTERFACE destroy
  fi


  Display 1 Tunnel tear down completed.

  exit 0
fi



##################
# Tunnel over V4 #
##################
if [ X"${TSP_TUNNEL_MODE}" = X"v6v4" ] || [ X"${TSP_TUNNEL_MODE}" = X"v6udpv4" ]; then


  if [ X"${TSP_HOST_TYPE}" = X"host" ] || [ X"${TSP_HOST_TYPE}" = X"router" ]; then
    #
    # Configured tunnel config (IPv4)
    Display 1 Setting up interface $TSP_TUNNEL_INTERFACE
    # first delete any previous tunnel
	
    if [ X"${TSP_TUNNEL_MODE}" = X"v6v4" ]; then
      # Check if interface exists and remove it
      $ifconfig $TSP_TUNNEL_INTERFACE >/dev/null 2>/dev/null
      if [ $? -eq 0 ]; then
        Exec $ifconfig $TSP_TUNNEL_INTERFACE destroy
      fi
      Exec $ifconfig $TSP_TUNNEL_INTERFACE create
      if [ $rel -eq 4 ]; then
        Exec $gifconfig $TSP_TUNNEL_INTERFACE $TSP_CLIENT_ADDRESS_IPV4 $TSP_SERVER_ADDRESS_IPV4         
      else
        Exec $ifconfig $TSP_TUNNEL_INTERFACE tunnel $TSP_CLIENT_ADDRESS_IPV4 $TSP_SERVER_ADDRESS_IPV4
      fi
    else 
      # Check if the interface already has an IPv6 configuration
      list=`$ifconfig $TSP_TUNNEL_INTERFACE | grep inet6 | awk '{print $2}' | grep -v '^fe80'`
      for ipv6address in $list
      do 
        Exec $ifconfig $TSP_TUNNEL_INTERFACE inet6 $ipv6address delete
      done
      Exec $ifconfig $TSP_TUNNEL_INTERFACE up
      fi
      #
      # Configured tunnel config (IPv6) 
	
      Exec $ifconfig $TSP_TUNNEL_INTERFACE inet6 $TSP_CLIENT_ADDRESS_IPV6 $TSP_SERVER_ADDRESS_IPV6 prefixlen $TSP_TUNNEL_PREFIXLEN alias
      Exec $ifconfig $TSP_TUNNEL_INTERFACE mtu 1280
      # 
      # Default route  
      Display 1 Adding default route to $TSP_SERVER_ADDRESS_IPV6
 
      # Delete any default IPv6 route first
      ExecNoCheck $route delete -inet6 default
      Exec $route add -inet6 default $TSP_SERVER_ADDRESS_IPV6
    fi
   #
   # DNS
   if [ X"${TSP_CLIENT_DNS_ADDRESS_IPV6}" != X"" ]; then
     Display 1 "Adding DNS server"
     Display 1 "NOTE: Adjust template script to perform actions"
     # ExecNoCheck cp ${resolv_conf} ${resolv_conf}.bak
     # echo "echo \"nameserver ${TSP_CLIENT_DNS_ADDRESS_IPV6}\" | cat - ${resolv_conf}.bak >${resolv_conf}"
     # echo "nameserver ${TSP_CLIENT_DNS_ADDRESS_IPV6}" | cat - ${resolv_conf}.bak >${resolv_conf}
   fi
	
    # Router configuration if required
    if [ X"${TSP_HOST_TYPE}" = X"router" ]; then
      Display 1 "Router configuration"
      Display 1 "Kernel setup"
      if [ X"${TSP_PREFIXLEN}" != X"64" ]; then
        ExecNoCheck $route add -inet6 $TSP_PREFIX::  -prefixlen $TSP_PREFIXLEN -interface lo0
      fi
      Exec $sysctl -w net.inet6.ip6.forwarding=1     # ipv6_forwarding enabled
      Exec $sysctl -w net.inet6.ip6.accept_rtadv=0   # routed must disable any incoming router advertisement
      Display 1 "Adding prefix to $TSP_HOME_INTERFACE"
      Exec $ifconfig $TSP_HOME_INTERFACE inet6 $TSP_PREFIX::1 prefixlen 64  
      # Router advertisement startup
      KillProcess rtadvd
      Exec $rtadvd $TSP_HOME_INTERFACE  
    fi

##################
# Tunnel over V6 #
##################
elif [ X"${TSP_TUNNEL_MODE}" = X"v4v6" ]; then


    if [ X"${TSP_HOST_TYPE}" = X"host" ] || [ X"${TSP_HOST_TYPE}" = X"router" ]; then
       #
       # Configured tunnel config (IPv4 or IPv6)
       Display 1 Setting up interface $TSP_TUNNEL_INTERFACE

       # Check if interface exists 
       $ifconfig $TSP_TUNNEL_INTERFACE >/dev/null 2>/dev/null
       if [ $? -eq 0 ]; then
	  # Interface exists, we destroy it completly instead of deconfiguring it
	   Exec $ifconfig $TSP_TUNNEL_INTERFACE destroy
       fi

       Exec $ifconfig $TSP_TUNNEL_INTERFACE create

       #configure v4v6 tunnel 
       Exec $ifconfig $TSP_TUNNEL_INTERFACE inet6 tunnel $TSP_CLIENT_ADDRESS_IPV6 $TSP_SERVER_ADDRESS_IPV6
       Exec $ifconfig $TSP_TUNNEL_INTERFACE $TSP_CLIENT_ADDRESS_IPV4/$TSP_TUNNEL_PREFIXLEN $TSP_SERVER_ADDRESS_IPV4


       #Validate if 1280 really is the good value here
       Exec $ifconfig $TSP_TUNNEL_INTERFACE mtu 1280

       Display 1 "deleting actual v4 default route"
       ExecNoCheck $route delete -inet default
       Display 1 Adding v4 default route to $TSP_SERVER_ADDRESS_IPV4
       Exec $route add default $TSP_SERVER_ADDRESS_IPV4

    fi

    # Router configuration if required
    if [ X"${TSP_HOST_TYPE}" = X"router" ]; then
       Display 1 "Router configuration"

       # TODO: Enable v4 forwarding

    fi

fi


Display 1 '--- End of configuration script. ---'

exit 0


