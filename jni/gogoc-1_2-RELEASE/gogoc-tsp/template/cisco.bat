@echo off
Rem #$Id
Rem
Rem Copyright (c) 2001-2003,2006,2007 gogo6 Inc. All rights reserved.
Rem For license information refer to CLIENT-LICENSE.TXT
Rem **************************************
Rem * Tunnel Server Protocol version 1.0 *
Rem * Host configuration script          *
Rem * For Cisco Routers                  *
Rem * Lanched from Windows               *
Rem **************************************
Rem * This script keeps a log

set LOG=NUL
set LOG2=NUL
set CISCO="%TSP_HOME_DIR%\CISCOIPv6.TXT"
if     %TSP_VERBOSE% == 2 set LOG2=CON
if not %TSP_VERBOSE% == 0 set LOG=CON

date /T > %LOG%
time /T > %LOG%

Rem ***** Script an IPv6 tunnel **********

:begin
echo IPv4 tunnel server address configured : %TSP_SERVER_ADDRESS_IPV4% > %LOG%
echo ! Add these lines to your Cisco configuration > %CISCO%
echo ! Script launched from a Microsoft environment >> %CISCO%
echo ! >> %CISCO%
echo ipv6 unicast-routing >> %CISCO%
echo ! >> %CISCO%
echo interface %TSP_TUNNEL_INTERFACE%  >> %CISCO%
echo  ipv6 address %TSP_CLIENT_ADDRESS_IPV6%/128 >> %CISCO%
echo  tunnel source %TSP_CLIENT_ADDRESS_IPv4% >> %CISCO%
echo  tunnel destination %TSP_SERVER_ADDRESS_IPV4% >> %CISCO%
echo  tunnel mode ipv6ip >> %CISCO%
echo ! >> %CISCO%
echo ipv6 route ::/0 %TSP_TUNNEL_INTERFACE% >> %CISCO%

if %TSP_HOST_TYPE% == router GOTO router_config
goto success

:router_config
echo cisco Router will be configured as IPv6 router and it will do router advertisements for autoconfiguration > %LOG%
echo Configuring IPv6_forwading on network interface > %LOG%
echo ! >> %CISCO%
echo interface %TSP_HOME_INTERFACE% >> %CISCO%
echo  ipv6 address %TSP_PREFIX%:1::/64 eui-64 >> %CISCO%
echo  ipv6 nd prefix-advertisement %TSP_PREFIX%:1::/64 43200 43200 onlink autoconfig >> %CISCO%
goto success

:success
Echo Cisco IPv6 configuration done and saved in this file : %CISCO%
Echo Look the file and apply these configuration lines to your Cisco configuration
goto end

:end
Echo End of the script

