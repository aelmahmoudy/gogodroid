#!/bin/sh
#
# $Id: openwrt.sh,v 1.3 2010/03/07 19:31:18 carl Exp $
#
# This source code copyright (c) gogo6 Inc. 2002-2007.
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
ipconfig=/usr/sbin/ip
rtadvd=/usr/sbin/radvd
rtadvd_pid=/var/run/radvd.pid
sysctl=/sbin/sysctl
rtadvdconfigfilename=gogoc-radvd.conf
rtadvdconfigfile=/var/run/gogoc/$rtadvdconfigfilename
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

  # Remove tunnel
  if [ X"${TSP_TUNNEL_MODE}" = X"v6v4" ]; then
    ExecNoCheck $ipconfig tunnel del $TSP_TUNNEL_INTERFACE
  fi

  # Remove addresses
  PREF=`echo $TSP_CLIENT_ADDRESS_IPV6 | sed "s/:0*/:/g" |cut -d : -f1-2`
  OLDADDR=`$ifconfig $TSP_TUNNEL_INTERFACE | grep "inet6.* $PREF" | sed -e "s/^.*inet6 addr: //" -e "s/ Scope.*\$//"`
  if [ ! -z $OLDADDR ]; then
    Display 1 "Removing old IPv6 address $OLDADDR"
    Exec $ifconfig $TSP_TUNNEL_INTERFACE inet6 del $OLDADDR
  fi

  # Bring interface down.
  ExecNoCheck $ifconfig $TSP_TUNNEL_INTERFACE down

  # Delete any IPv6 route
  ExecNoCheck $route -A inet6 del ::/0 2>/dev/null # delete old default route
  ExecNoCheck $route -A inet6 del 2000::/3 2>/dev/null  # delete old gw route


  # Router deconfiguration.
  if [ X"${TSP_HOST_TYPE}" = X"router" ]; then
    # Remove prefix routing on TSP_HOME_INTERFACE
    ExecNoCheck $route -A inet6 del $TSP_PREFIX::/$TSP_PREFIXLEN

    # Remove address from TSP HOME INTERFACE
    OLDADDR=`$ifconfig $TSP_HOME_INTERFACE | grep "inet6.* $PREF" | sed -e "s/^.*inet6 addr: //" -e "s/ Scope.*\$//"`
    if [ ! -z $OLDADDR ]; then
      Display 1 "Removing old IPv6 address $OLDADDR"
      Exec $ifconfig $TSP_HOME_INTERFACE del $OLDADDR
    fi

    # Kill router advertisement daemon
    /etc/init.d/S51radvd stop
  fi


  Display 1 Tunnel tear down completed.

  exit 0
fi


if [ X"${TSP_HOST_TYPE}" = X"host" ] || [ X"${TSP_HOST_TYPE}" = X"router" ]; then
   #
   # Configured tunnel config (IPv6) 
   Display 1 "$TSP_TUNNEL_INTERFACE setup"
   if [ X"${TSP_TUNNEL_MODE}" = X"v6v4" ]; then
      Display 1 "Setting up link to $TSP_SERVER_ADDRESS_IPV4"
	ExecNoCheck $ipconfig tunnel del $TSP_TUNNEL_INTERFACE
	ExecNoCheck sleep 1
        Exec $ipconfig tunnel add $TSP_TUNNEL_INTERFACE mode sit ttl 64 remote $TSP_SERVER_ADDRESS_IPV4
   fi

   Exec $ifconfig $TSP_TUNNEL_INTERFACE up

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
   Display 1 "Kernel setup"
   if [ X"${TSP_PREFIXLEN}" != X"64" ]; then
      #Better way on linux to avoid loop with the remaining /48?
      $route -A inet6 add $TSP_PREFIX::/$TSP_PREFIXLEN dev $TSP_HOME_INTERFACE 2>/dev/null
   fi
   Display 1 "Adding prefix to $TSP_HOME_INTERFACE"
   OLDADDR=`$ifconfig $TSP_HOME_INTERFACE | grep "inet6.* $PREF" | sed -e "s/^.*inet6 addr: //" -e "s/ Scope.*\$//"`
   if [ ! -z $OLDADDR ]; then
      Display 1 "Removing old IPv6 address $OLDADDR"
      Exec $ifconfig $TSP_HOME_INTERFACE del $OLDADDR
   fi
   Exec $ifconfig $TSP_HOME_INTERFACE add $TSP_PREFIX::1/64
   # Router advertisement configuration 
   Display 1 "Create new $rtadvdconfigfile"
   echo "##### rtadvd.conf made by gogoCLIENT ####" > "$rtadvdconfigfile"
   echo "interface $TSP_HOME_INTERFACE" >> "$rtadvdconfigfile"
   echo "{" >> "$rtadvdconfigfile"
   echo " AdvSendAdvert on;" >> "$rtadvdconfigfile"
   echo " prefix $TSP_PREFIX::/64" >> "$rtadvdconfigfile"
   echo " {" >> "$rtadvdconfigfile"
   echo " AdvOnLink on;" >> "$rtadvdconfigfile"
   echo " AdvAutonomous on;" >> "$rtadvdconfigfile"
   echo " };" >> "$rtadvdconfigfile"
   echo "};" >> "$rtadvdconfigfile"
   echo "" >> "$rtadvdconfigfile"
   /etc/init.d/S51radvd stop
   # Then enable forwarding. Killing
   # radvd disables forwarding and radvd
   # does NOT start without forwarding enabled
   Exec $sysctl -w net.ipv6.conf.all.forwarding=1 # ipv6_forwarding enabled  
   if [ -f $rtadvdconfigfile ]; then
      Exec $rtadvd -p $rtadvd_pid -C $rtadvdconfigfile
      Display 1 "Starting radvd: $rtadvd -p $rtadvd_pid -C $rtadvdconfigfile"
   else
      echo "Error : file $rtadvdconfigfile not found"
      exit 1
   fi
fi

Display 1 "--- End of configuration script. ---"

exit 0

#---------------------------------------------------------------------
