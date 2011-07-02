#!/bin/sh
#
# $Id: linux.sh,v 1.3 2010/03/07 19:31:17 carl Exp $
#
# This source code copyright (c) gogo6 Inc. 2002-2006.
#
# For license information refer to CLIENT-LICENSE.TXT
#
# Note: IPV6 support and tun Support must be enabled before calling this script.
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
   echo "$*"
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

Display 1 "--- Start of configuration script. ---"
Display 1 "Script: " `basename $0`

ifconfig=/sbin/ifconfig
route=/sbin/route
ipconfig=/sbin/ip
rtadvd=/usr/sbin/radvd
rtadvd_pid=/var/run/radvd/radvd.pid
sysctl=/sbin/sysctl
rtadvdconfigfilename=gogoc-rtadvd.conf
rtadvdconfigfile=$TSP_HOME_DIR/$rtadvdconfigfilename
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


#############################################################################
# Tunnel destruction script.
#
#   Is invoked by the gogoCLIENT on shutdown when it receives the 
#   SIGHUP signal. Use "kill -HUP <gogoc pid>".
#
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

    # Kill router advertisement daemon
    KillProcess $rtadvdconfigfile

    # Remove prefix routing on TSP_HOME_INTERFACE
    ExecNoCheck $route -A inet6 del $TSP_PREFIX::/$TSP_PREFIXLEN

    # Remove Blackhole.
    if [ X"${TSP_PREFIXLEN}" != X"64" ]; then
      ExecNoCheck $route -A inet6 del $TSP_PREFIX::/$TSP_PREFIXLEN dev lo
    fi

    # Remove address from TSP HOME INTERFACE
    ExecNoCheck $ifconfig $TSP_HOME_INTERFACE inet6 del $TSP_PREFIX::1/64
  fi

  # Delete default IPv6 route(s).
  ExecNoCheck $route -A inet6 del ::/0     2>/dev/null  # delete default route
  ExecNoCheck $route -A inet6 del 2000::/3 2>/dev/null  # delete gw route

  # Destroy tunnel interface
  if [ -x $ipconfig ]; then
    # Delete tunnel via ipconfig
    ExecNoCheck $ipconfig tunnel del $TSP_TUNNEL_INTERFACE
  else  
    # Check if interface exists and remove it
    $ifconfig $TSP_TUNNEL_INTERFACE >/dev/null 2>/dev/null
    if [ $? -eq 0 ]; then

      Delete interface IPv6 configuration.
      PREF=`echo $TSP_CLIENT_ADDRESS_IPV6 | sed "s/:0*/:/g" |cut -d : -f1-2`
      OLDADDR=`$ifconfig $TSP_TUNNEL_INTERFACE | grep "inet6.* $PREF" | sed -e "s/^.*inet6 addr: //" -e "s/ Scope.*\$//"`
      if [ ! -z $OLDADDR ]; then
        ExecNoCheck $ifconfig $TSP_TUNNEL_INTERFACE inet6 del $OLDADDR
      fi

      # Bring interface down
      ExecNoCheck $ifconfig $TSP_TUNNEL_INTERFACE down
    fi
  fi
  

  Display 1 Tunnel tear down completed.

  exit 0
fi


#############################################################################
# Tunnel creation script.
#
if [ X"${TSP_HOST_TYPE}" = X"host" ] || [ X"${TSP_HOST_TYPE}" = X"router" ]; then

   # Set tunnel IPv6 configuration
   Display 1 "$TSP_TUNNEL_INTERFACE setup"
   if [ X"${TSP_TUNNEL_MODE}" = X"v6v4" ]; then
      Display 1 "Setting up link to $TSP_SERVER_ADDRESS_IPV4"
      if [ -x $ipconfig ]; then
	 ExecNoCheck $ipconfig tunnel del $TSP_TUNNEL_INTERFACE
	 ExecNoCheck sleep 1
         Exec $ipconfig tunnel add $TSP_TUNNEL_INTERFACE mode sit ttl 64 remote $TSP_SERVER_ADDRESS_IPV4
      else
         Exec $ifconfig $TSP_TUNNEL_INTERFACE tunnel ::$TSP_SERVER_ADDRESS_IPV4
      fi
   fi

   Exec $ifconfig $TSP_TUNNEL_INTERFACE up

   # Clean-up old interface IPv6 configuration.
   PREF=`echo $TSP_CLIENT_ADDRESS_IPV6 | sed "s/:0*/:/g" |cut -d : -f1-2`
   OLDADDR=`$ifconfig $TSP_TUNNEL_INTERFACE | grep "inet6.* $PREF" | sed -e "s/^.*inet6 addr: //" -e "s/ Scope.*\$//"`
   if [ ! -z $OLDADDR ]; then
      Display 1 "Removing old IPv6 address $OLDADDR"
      Exec $ifconfig $TSP_TUNNEL_INTERFACE inet6 del $OLDADDR
   fi

   Display 1 "This host is: $TSP_CLIENT_ADDRESS_IPV6/$TSP_TUNNEL_PREFIXLEN"
   Exec $ifconfig $TSP_TUNNEL_INTERFACE add $TSP_CLIENT_ADDRESS_IPV6/$TSP_TUNNEL_PREFIXLEN
   Exec $ifconfig $TSP_TUNNEL_INTERFACE mtu 1280

   # 
   # Default route  
   Display 1 "Adding default route"
   ExecNoCheck $route -A inet6 del ::/0 2>/dev/null # delete old default route
   ExecNoCheck $route -A inet6 del 2000::/3 2>/dev/null  # delete old gw route
   Exec $route -A inet6 add ::/0 dev $TSP_TUNNEL_INTERFACE
   Exec $route -A inet6 add 2000::/3 dev $TSP_TUNNEL_INTERFACE
   #
   # DNS
   if [ X"${TSP_CLIENT_DNS_ADDRESS_IPV6}" != X"" ]; then
     Display 1 "Adding DNS server"
     Display 1 "NOTE: Adjust template script to perform actions"
     # ExecNoCheck cp ${resolv_conf} ${resolv_conf}.bak
     # echo "echo \"nameserver ${TSP_CLIENT_DNS_ADDRESS_IPV6}\" | cat - ${resolv_conf}.bak >${resolv_conf}"
     # echo "nameserver ${TSP_CLIENT_DNS_ADDRESS_IPV6}" | cat - ${resolv_conf}.bak >${resolv_conf}
   fi
fi

# Router configuration if required
if [ X"${TSP_HOST_TYPE}" = X"router" ]; then

   Display 1 "Router configuration"

   # Tell kernel to forward IPv6 traffic.
   Exec $sysctl -w net.ipv6.conf.all.forwarding=1

   # Blackholing on interface lo, if prefixlen is not 64.
   if [ X"${TSP_PREFIXLEN}" != X"64" ]; then
     # Sometimes this route does not show when using 'netstat -rn6'.
     ExecNoCheck $route -A inet6 add $TSP_PREFIX::/$TSP_PREFIXLEN dev lo 2>/dev/null
   fi

   # Add prefix::1 on advertising interface. Clean up before.
   OLDADDR=`$ifconfig $TSP_HOME_INTERFACE | grep "inet6.* $PREF" | sed -e "s/^.*inet6 addr: //" -e "s/ Scope.*\$//"`
   if [ ! -z $OLDADDR ]; then
      Display 1 "Removing old IPv6 address $OLDADDR"
      Exec $ifconfig $TSP_HOME_INTERFACE inet6 del $OLDADDR
   fi
   Display 1 "Adding prefix to $TSP_HOME_INTERFACE"
   Exec $ifconfig $TSP_HOME_INTERFACE add $TSP_PREFIX::1/64


   # Stop radvd daemon if it was running. Twice.
   /etc/init.d/radvd stop
   if [ -f $rtadvdconfigfile ]; then
     KillProcess $rtadvdconfigfile
   fi

   # Create new radvd configuration file.
   cat > "$rtadvdconfigfile" <<EOF
##### rtadvd.conf made by gogoCLIENT ####
interface $TSP_HOME_INTERFACE
{
  AdvSendAdvert on;
  AdvLinkMTU 1280;
  prefix $TSP_PREFIX::/64
  {
    AdvOnLink on;
    AdvAutonomous on;
  };
};
EOF

   # Start the radvd daemon.
   Display 1 "Starting radvd: $rtadvd -u radvd -C $rtadvdconfigfile"
   Exec $rtadvd -u radvd -p $rtadvd_pid -C $rtadvdconfigfile
fi

Display 1 "--- End of configuration script. ---"

exit 0

#---------------------------------------------------------------------
