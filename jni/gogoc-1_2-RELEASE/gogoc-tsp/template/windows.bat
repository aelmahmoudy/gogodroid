@echo off
rem
rem #$Id: windows.bat,v 1.1 2009/11/20 16:53:45 jasminko Exp $
rem
rem This source code copyright (c) gogo6 Inc. 2002-2007.
rem
rem For license information refer to CLIENT-LICENSE.TXT
rem 
rem *********************************************************************
rem *   Tunnel Server Protocol version 2.0.1                            *
rem *   Host configuration script                                       *
rem *   Windows 2000, XP, 2003 and Vista are supported                  *
rem *********************************************************************
rem * This script keeps a log
rem

set LOG=NUL
set LOG2=NUL
if     "%TSP_VERBOSE%" == "3" set LOG2=CON
if not "%TSP_VERBOSE%" == "0" set LOG=CON

echo Script execution time: %DATE%, %TIME%

set NEWSTACK=NO
set WINDOWS_VERSION=0

REM This is the name of the v6v4 we will spawn under XP sp1 & sp2
set TEMPLATE_V6V4_TUNNEL_INTERFACE=gogo6_tunv6


REM *** ----------------------------------------------- ***
REM *** Get the windows verion                          ***
echo Testing Windows NT version


Rem OS might have been preseted
if X%TSP_CLIENT_OS% == X goto testver

Echo Got the OS value %TSP_CLIENT_OS% preset from the broker
if %TSP_CLIENT_OS% == winXPSP1 set WINDOWS_VERSION=1
if %TSP_CLIENT_OS% == winXPSP2 set WINDOWS_VERSION=2
if %TSP_CLIENT_OS% == winXP set WINDOWS_VERSION=3
if %TSP_CLIENT_OS% == win2000 set WINDOWS_VERSION=4
if not %WINDOWS_VERSION% == 0 goto donttestver


REM Use internal tool to determine version
:testver
"%TSP_HOME_DIR%\template\win-ver.exe"
set WINDOWS_VERSION=%errorlevel%


REM *** ----------------------------------------------- ***
REM *** Verify if IPv6 is installed and working on host ***
:donttestver
echo Testing IPv6 presence

if %WINDOWS_VERSION% == 5 goto test_ipv6_ping
if %WINDOWS_VERSION% == 6 goto test_ipv6_ping
if %WINDOWS_VERSION% == 7 goto test_ipv6_ping

ping6 -n 1 ::1 > NUL
if errorlevel 1 goto fail0
goto set_interface

:test_ipv6_ping
ping -6 -n 1 ::1 > NUL
if errorlevel 1 goto fail10


REM *** ----------------------------------------------- ***
REM *** Set up the interface                            ***
:set_interface
if %WINDOWS_VERSION% == 1 goto windows_xp_SP1
if %WINDOWS_VERSION% == 2 goto windows_xp_SP2
if %WINDOWS_VERSION% == 3 goto windows_xp
if %WINDOWS_VERSION% == 4 goto windows_2000
if %WINDOWS_VERSION% == 5 goto windows_server_2003_SP1
if %WINDOWS_VERSION% == 6 goto windows_server_2003
if %WINDOWS_VERSION% == 7 goto windows_vista


REM Windows XP SP1 and later script path.
:windows_vista
:windows_server_2003_SP1
:windows_server_2003
:windows_xp_SP2
:windows_xp_SP1

REM Set what we change to be non-persistant.
set STORAGEARG=store=active


:windows_xp
echo Cycling the interface
netsh.exe interface ipv6 delete interface %TEMPLATE_V6V4_TUNNEL_INTERFACE% > NUL

Rem are we setting up nat traversal or V6V4 or V4V6?
if %TSP_TUNNEL_MODE% == v6udpv4 goto v6udpv4_config
if %TSP_TUNNEL_MODE% == v4v6 goto v4v6_config

Rem else, setup v6v4
Rem can we spawn our own GIF interface on this os?
if %WINDOWS_VERSION% == 1 goto create_gif
if %WINDOWS_VERSION% == 2 goto create_gif
REM skip xp and 2000
if %WINDOWS_VERSION% == 5 goto create_gif
if %WINDOWS_VERSION% == 6 goto create_gif
if %WINDOWS_VERSION% == 7 goto create_gif
goto older_xp


:create_gif
echo Configuring V6V4 for XP Service Pack 1 and newer
echo Overriding TSP_TUNNEL_INTERFACE from %TSP_TUNNEL_INTERFACE% to %TEMPLATE_V6V4_TUNNEL_INTERFACE%
set TSP_TUNNEL_INTERFACE=%TEMPLATE_V6V4_TUNNEL_INTERFACE%

netsh.exe interface ipv6 add v6v4tunnel %TSP_TUNNEL_INTERFACE% %TSP_CLIENT_ADDRESS_IPV4% %TSP_SERVER_ADDRESS_IPV4% > NUL
if errorlevel 1 goto fail9

netsh.exe interface ipv6 add address %TSP_TUNNEL_INTERFACE% %TSP_CLIENT_ADDRESS_IPV6% %STORAGEARG% > NUL
if errorlevel 1 goto fail2

netsh.exe interface ipv6 add route ::/0 %TSP_TUNNEL_INTERFACE% publish=yes %STORAGEARG% > NUL
if errorlevel 1 goto fail1

goto set_mtu


:older_xp
echo Configuring V6V4 for XP with no service pack
netsh.exe interface ipv6 add route prefix=::/0 interface=2 nexthop=::%TSP_SERVER_ADDRESS_IPV4% publish=yes > NUL
if errorlevel 1 goto fail1

netsh.exe interface ipv6 add address interface=2 address=%TSP_CLIENT_ADDRESS_IPV6% > NUL
if errorlevel 1 goto fail2


:set_mtu
Rem We skip mtu setting if in windows XP without any SP, it doesnt support it
if %WINDOWS_VERSION% == 3 goto check_router_config
echo Setting MTU to 1280 on tunnel interface "%TSP_TUNNEL_INTERFACE%"
netsh.exe interface ipv6 set interface interface="%TSP_TUNNEL_INTERFACE%" mtu=1280 > NUL
if errorlevel 1 goto fail8


:check_router_config
if %TSP_HOST_TYPE% == router GOTO router_config
goto success


:v6udpv4_config
echo Configuring for V6UDPV4 (NAT traversal)

netsh.exe interface ipv6 set address "%TSP_TUNNEL_INTERFACE%" %TSP_CLIENT_ADDRESS_IPV6% %STORAGEARG% > NUL
if errorlevel 1 goto fail3

Rem We skip mtu setting if in windows XP without any SP, it doesnt support it
if %WINDOWS_VERSION% == 3 goto skip_set_mtu
echo Setting MTU to 1280 on tunnel interface "%TSP_TUNNEL_INTERFACE%"
netsh.exe interface ipv6 set interface interface="%TSP_TUNNEL_INTERFACE%" mtu=1280 %STORAGEARG% > NUL
if errorlevel 1 goto fail8


IF NOT %WINDOWS_VERSION% == 7 GOTO skip_vista_dns
REM Windows Vista -ONLY-
REM Add DNS servers to the tunnel interface.
ECHO Adding DNS servers to tunnel interface "%TSP_TUNNEL_INTERFACE%"
IF NOT "%TSP_LOCAL_DNS01%" == "" netsh interface ipv4 add dnsserver name="%TSP_TUNNEL_INTERFACE%" addr=%TSP_LOCAL_DNS01% > NUL
IF NOT "%TSP_LOCAL_DNS02%" == "" netsh interface ipv4 add dnsserver name="%TSP_TUNNEL_INTERFACE%" addr=%TSP_LOCAL_DNS02% > NUL
:skip_vista_dns


:skip_set_mtu
netsh.exe interface ipv6 add route %TSP_SERVER_ADDRESS_IPV6%/128 "%TSP_TUNNEL_INTERFACE%" %STORAGEARG% > NUL
if NOT %WINDOWS_VERSION% == 7 if errorlevel 1 goto fail3

netsh.exe interface ipv6 add route ::/0 "%TSP_TUNNEL_INTERFACE%" %TSP_SERVER_ADDRESS_IPV6% publish=yes %STORAGEARG% > NUL
if NOT %WINDOWS_VERSION% == 7 if errorlevel 1 goto fail3

if %TSP_HOST_TYPE% == router GOTO router_config
goto success


:v4v6_config
echo Configuring for V4V6, %TSP_CLIENT_ADDRESS_IPV4% to %TSP_SERVER_ADDRESS_IPV4% on %TSP_TUNNEL_INTERFACE%...

netsh.exe interface ip set address "%TSP_TUNNEL_INTERFACE%" static %TSP_CLIENT_ADDRESS_IPV4% 255.255.255.0 > NUL
route.exe add %TSP_SERVER_ADDRESS_IPV4% mask 255.255.255.255 %TSP_CLIENT_ADDRESS_IPV4%
route.exe add 0.0.0.0 mask 0.0.0.0 %TSP_SERVER_ADDRESS_IPV4%
goto success_v4v6


:router_config
ECHO Preparing router configuration...
netsh.exe interface ipv6 set address "%TSP_HOME_INTERFACE%" %TSP_PREFIX%::1 %STORAGEARG% > NUL
if errorlevel 1 goto fail6

echo Configuring IPv6 forwading on internal and external interfaces 
netsh.exe interface ipv6 set interface interface="%TSP_TUNNEL_INTERFACE%" forwarding=enabled %STORAGEARG% > NUL
netsh.exe interface ipv6 set interface interface="%TSP_HOME_INTERFACE%" forwarding=enabled %STORAGEARG% > NUL
netsh.exe interface ipv6 set interface interface=2 forwarding=enabled %STORAGEARG%  > NUL

Rem fix for XP with no service pack. forwarding only works if enabled 
Rem on the 6to4 interface. 
if %WINDOWS_VERSION% == 3 netsh.exe interface ipv6 set interface interface=3 forwarding=enabled %STORAGEARG% > NUL

ECHO Configuring this computer to send router advertisements on interface "%TSP_HOME_INTERFACE%"
netsh.exe interface ipv6 set interface "%TSP_HOME_INTERFACE%" advertise=enabled %STORAGEARG% > NUL
if errorlevel 1 goto fail7

echo You got the IPv6 prefix : %TSP_PREFIX%/%TSP_PREFIXLEN% 
netsh.exe interface ipv6 add route prefix=%TSP_PREFIX%::/64 interface="%TSP_HOME_INTERFACE%" siteprefixlength=64 publish=yes %STORAGEARG% > NUL
if NOT %WINDOWS_VERSION% == 7 if errorlevel 1 goto fail4
goto success


Rem *** Windows 2000 script path ***

:windows_2000
if %TSP_TUNNEL_MODE% == v6udpv4 goto fail3
echo IPv4 tunnel server address configured : %TSP_SERVER_ADDRESS_IPV4%
ipv6.exe rtu ::/0 2/::%TSP_SERVER_ADDRESS_IPV4% pub
if errorlevel 1 goto fail1

echo IPv6 host address configured : %TSP_CLIENT_ADDRESS_IPV6% 
ipv6.exe adu 2/%TSP_CLIENT_ADDRESS_IPV6% 
if errorlevel 1 goto fail2
if %TSP_HOST_TYPE% == router GOTO router_config_2000
goto success

:router_config_2000
echo This computer will be configured as IPv6 router and it will do router advertisements for autoconfiguration > %LOG%
echo Configuring IPv6_forwarding

Rem we turn on forwarding on all interfaces here
Rem to address the fact we need to enable it
Rem on the 6to4 interface and its interface
Rem number is variable

ipv6.exe ifc %TSP_HOME_INTERFACE% forwards
ipv6.exe ifc 2 forwards  
ipv6.exe ifc 3 forwards 
ipv6.exe ifc 4 forwards 
ipv6.exe ifc 5 forwards 
ipv6.exe ifc 6 forwards 
ipv6.exe ifc 7 forwards 
ipv6.exe ifc 8 forwards

echo Configuring routing advertisement on the specified interface 
ipv6.exe ifc %TSP_HOME_INTERFACE% advertises

echo You got the IPv6 prefix : %TSP_PREFIX%::/%TSP_PREFIXLEN% 
echo Your network interface %TSP_HOME_INTERFACE% will advertise %TSP_PREFIX%::/64 
ipv6.exe rtu %TSP_PREFIX%::/64 %TSP_HOME_INTERFACE% publish life 86400 spl 64 
if errorlevel 1 goto fail4
echo Address %TSP_PREFIX%::1 confired on interface %TSP_HOME_INTERFACE%
ipv6.exe adu %TSP_HOME_INTERFACE%/%TSP_PREFIX%::1 
goto success

:fail0
echo Failed : IPv6 not installed. Use the command "ipv6 install" to enable it. 
exit 1
goto end

:fail1
Echo Failed : cannot use the IPv4 tunnel server address 
exit 1
goto end

:fail2
Echo Failed : cannot use the IPv6 host address 
exit 1
goto end

:fail3
Echo Failed : cannot setup nat traversal 
exit 1
goto end

:fail4
Echo Failed : cannot assign the %TSP_PREFIX% route on interface %TSP_HOME_INTERFACE%
exit 1
goto end

:fail5 
Echo Failed : cannot disable the firewall on the outgoing interface 
exit 1
goto end

:fail6 
Echo Failed : cannot configure %TSP_PREFIX% on the %TSP_HOME_INTERFACE% interface 
exit 1
goto end

:fail7 
Echo Failed : cannot advertise the prefix %TSP_PREFIX%/64 on interface %TSP_HOME_INTERFACE% 
exit 1
goto end

:fail8
Echo Failed : cannot set the MTU to 1280 on the tunnel interface 
exit 1
goto end

:fail9
Echo Failed : cannot create a new v6v4 tunnel interface
exit 1
goto end

:fail10
Echo Failed : IPv6 not installed. Use the command "netsh interface ipv6 install" to enable it. 
exit 1
goto end

:success
Echo Success ! Now, you're ready to use IPv6 connectivity to Internet IPv6
if %TSP_HOST_TYPE% == router GOTO success2
goto end

:success2
Echo The tunnel server uses this IPv6 address : %TSP_SERVER_ADDRESS_IPV6%
Echo The prefix advertised is %TSP_PREFIX%/64 on interface %TSP_HOME_INTERFACE%
goto end

:success_v4v6
Echo Success! Now, you're ready to use IPv4 over IPv6!
goto end

:end

Echo End of the script


