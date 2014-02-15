@ECHO OFF
REM #########################################################################
REM $Id: windows.cmd,v 1.3 2010/03/07 19:31:18 carl Exp $
REM
REM This source code copyright (c) gogo6 Inc. 2002-2007.
REM
REM   For license information refer to CLIENT-LICENSE.TXT
REM
REM Description:  
REM   This is the Windows version template script used by the gogoCLIENT
REM   to set(or destroy) up the negotiated tunnel. This script has multiple
REM   input  parameters that are set in environment variables. Here's a short 
REM   description for every variable set:
REM
REM   TSP_TUNNEL_MODE:         Tunnel mode (e.g.: v6v4, v6udpv4, v4v6)
REM   TSP_HOST_TYPE:           Host type (e.g.: host or router)
REM   TSP_TUNNEL_INTERFACE:    Tunneling interface to use (v6udpv4 & v4v6).
REM   TSP_HOME_INTERFACE:      Interface used for router advertisements.
REM   TSP_CLIENT_ADDRESS_IPV4: Client IPv4 local endpoint address.
REM   TSP_CLIENT_ADDRESS_IPV6: Client IPv6 local endpoint address.
REM   TSP_CLIENT_DNS_NAME:     Delegated user domain.
REM   TSP_SERVER_ADDRESS_IPV4: Server IPv4 remote endpoint address.
REM   TSP_SERVER_ADDRESS_IPV6: Server IPv6 remote endpoint address.
REM   TSP_TUNNEL_PREFIXLEN:    Tunnel address length (e.g.: 128 or 32)
REM   TSP_PREFIX:              Delegated prefix.
REM   TSP_PREFIXLEN:           Delegated prefix length.
REM   TSP_VERBOSE:             Verbosity level. Always 3. Not used.
REM   TSP_HOME_DIR:            gogoCLIENT Installation directory.
REM   TSP_LOCAL_DNS[XX]:       Local configured DNS server(s).
REM   TSP_OPERATION:           The operation requested (CREATE or TEARDOWN)
REM
REM   This script officially supoprts the following Windows versions:
REM     - Windows XP SP2 (32 and 64-bits), 
REM     - Windows 2003 Server (32 and 64-bits),
REM     - Windows Vista (32 and 64-bits),
REM   This script may work on other versions of Windows, but it has not been
REM   tested.
REM
REM Author: Charles Nepveu
REM
REM Date created: June 2007
REM
REM #########################################################################


REM Constant definitions, and variable initialisation.
SET V6V4_STATIC_INTERFACE=gogo6_tunv6
SET NETSH_COMMAND_SCRIPT="%TSP_HOME_DIR%\netsh_tun_cmd.txt"
SET NETSH_PERS=store=active
SET ERRNO=0


REM Timestamp script start time.
ECHO Script execution time: %DATE%, %TIME%


REM Verify if the proper environment variables are set. If they're not, 
REM it's probably because this script was not invoked from the gogoCLIENT.
IF "%TSP_HOME_DIR%" == ""            GOTO PROC_ERR
IF "%TSP_SERVER_ADDRESS_IPV4%" == "" GOTO PROC_ERR


REM Determine Windows version.
CALL :GET_WIN_VER_PROCEDURE
IF %ERRNO% NEQ 0 GOTO PROC_ERR


REM Verify if Windows version is supported.
CALL :VERI_VER_PROCEDURE
IF %ERRNO% NEQ 0 GOTO PROC_ERR


REM Check if IPv6 is functionnal on local computer.
CALL :IPV6_TEST_PROCEDURE
IF %ERRNO% NEQ 0 GOTO PROC_ERR


REM Check what operation we're attempting to do (CREATE, TEARDOWN).
IF /i "%TSP_OPERATION%" == "TSP_TUNNEL_CREATION" CALL :OP_CREATE_TUN_PROCEDURE
IF /i "%TSP_OPERATION%" == "TSP_TUNNEL_TEARDOWN" CALL :OP_TEARDOWN_TUN_PROCEDURE
IF %ERRNO% NEQ 0 GOTO PROC_ERR


REM Successful completion.
GOTO :END_SUCCESS


REM -------------------------------------------------------------------------

:GET_WIN_VER_PROCEDURE
REM Retrieves the Windows version and sets it in WIN_VER variable.

REM Windows version _can_ be provided by the environment variable 
REM TSP_CLIENT_OS.
IF "%TSP_CLIENT_OS%"  == ""           GOTO :RUN_WIN_VER
IF /i %TSP_CLIENT_OS% == winxpsp2     SET WIN_VER=2  &&  GOTO :EOF
IF /i %TSP_CLIENT_OS% == winxp64      SET WIN_VER=6  &&  GOTO :EOF
IF /i %TSP_CLIENT_OS% == winxp64sp1   SET WIN_VER=5  &&  GOTO :EOF
IF /i %TSP_CLIENT_OS% == winxp64sp2   SET WIN_VER=8  &&  GOTO :EOF
IF /i %TSP_CLIENT_OS% == win2003      SET WIN_VER=6  &&  GOTO :EOF
IF /i %TSP_CLIENT_OS% == win2003sp1   SET WIN_VER=5  &&  GOTO :EOF
IF /i %TSP_CLIENT_OS% == win2003sp2   SET WIN_VER=8  &&  GOTO :EOF
IF /i %TSP_CLIENT_OS% == winvista     SET WIN_VER=7  &&  GOTO :EOF
IF /i %TSP_CLIENT_OS% == winxpsp3     SET WIN_VER=9  &&  GOTO :EOF
GOTO :EOF

:RUN_WIN_VER
REM Run the win-ver executable and retrieve Windows version from errorlevel.
"%TSP_HOME_DIR%\win-ver.exe"
SET WIN_VER=%ERRORLEVEL%

GOTO :EOF


REM -------------------------------------------------------------------------

:VERI_VER_PROCEDURE
REM Verifies if the Windows version detected earlier is supported in this
REM script.

REM Verify if Windows version was identified.
IF "%WIN_VER%" == "" (
  SET ERRNO=10
  GOTO :EOF
)

REM 1 = Windows XP SP1
REM 2 = Windows XP SP2
REM 3 = Windows XP
REM 4 = Windows 2000
REM 5 = Windows Server 2003 SP1 / XP 64 SP1
REM 6 = Windows Server 2003     / XP 64
REM 7 = Windows Vista
REM 8 = Windows Server 2003 SP2 / XP 64 SP2
REM 9 = Windows XP SP3
IF %WIN_VER% EQU 2 GOTO :EOF
IF %WIN_VER% EQU 5 GOTO :EOF
IF %WIN_VER% EQU 6 GOTO :EOF
IF %WIN_VER% EQU 7 GOTO :EOF
IF %WIN_VER% EQU 8 GOTO :EOF
IF %WIN_VER% EQU 9 GOTO :EOF

REM Version is not supported
SET ERRNO=11

GOTO :EOF


REM -------------------------------------------------------------------------

:IPV6_TEST_PROCEDURE
REM Runs a simple test on the local computer to verify IPv6 connectivity.

PING -6 -n 1 ::1 > NUL
IF ERRORLEVEL 1 SET ERRNO=12
GOTO :EOF


REM -------------------------------------------------------------------------

:OP_CREATE_TUN_PROCEDURE
REM This is the path for tunnel creation


REM Delete existing V6V4 builtin interface if it is still there.
REM In case it was not deleted.
REM  TODO: Should perform entire cleanup.
netsh int ipv6 dele int %V6V4_STATIC_INTERFACE% > NUL


REM Set up the tunnel, with the negotiated parameters.
IF /i %TSP_TUNNEL_MODE% == v6v4     CALL :V6V4_TUN_PROCEDURE
IF /i %TSP_TUNNEL_MODE% == v6udpv4  CALL :V6UDPV4_TUN_PROCEDURE
IF /i %TSP_TUNNEL_MODE% == v4v6     CALL :V4V6_TUN_PROCEDURE
IF /i %ERRNO% NEQ 0 GOTO :EOF


REM Perform router configuration, if host type is router.
IF /i %TSP_HOST_TYPE% == router     CALL :ROUTER_CONFIG_PROCEDURE
IF %ERRNO% NEQ 0 GOTO :EOF


REM Finished tunnel setup.
GOTO :EOF


REM -------------------------------------------------------------------------

:V6V4_TUN_PROCEDURE
REM Sets up the V6V4 tunnel with the built-in tunnel interface.

REM Override tunnel interface with static one.
SET TSP_TUNNEL_INTERFACE=%V6V4_STATIC_INTERFACE%

ECHO Setting up a V6V4 tunnel using built-in windows v6v4tunnel.
ECHO Tunnel interface is: "%TSP_TUNNEL_INTERFACE%"

REM Create the v6v4 tunnel interface.
ECHO Creating the v6v4 tunnel interface ...
netsh int ipv6 add v6v4tunnel "%TSP_TUNNEL_INTERFACE%" %TSP_CLIENT_ADDRESS_IPV4% %TSP_SERVER_ADDRESS_IPV4% > NUL
IF ERRORLEVEL 1 ( 
  SET ERRNO=20
  GOTO :EOF
)

REM Add the local IPv6 address on the v6v4 tunnel interface.
ECHO Adding IPv6 address to the v6v4 tunnel interface ...
netsh int ipv6 add addr "%TSP_TUNNEL_INTERFACE%" %TSP_CLIENT_ADDRESS_IPV6% %NETSH_PERS% > NUL
IF ERRORLEVEL 1 ( 
  SET ERRNO=21
  GOTO :EOF
)

REM Route all IPv6 traffic on the v6v4 tunnel interface.
ECHO Routing all IPv6 traffic through the v6v4 tunnel interface ...
netsh int ipv6 add route ::/0 "%TSP_TUNNEL_INTERFACE%" publish=yes %NETSH_PERS% > NUL
IF ERRORLEVEL 1 ( 
  SET ERRNO=22
  GOTO :EOF
)

REM Set the MTU to 1280 on the v6v4 tunnel interface.
ECHO Setting MTU to 1280 on v6v4 tunnel interface ...
netsh int ipv6 set int "%TSP_TUNNEL_INTERFACE%" MTU=1280 > NUL
IF ERRORLEVEL 1 ( 
  SET ERRNO=23
  GOTO :EOF
)

REM Set the DNS server on the v6v4 tunnel interface.
IF NOT "%TSP_CLIENT_DNS_ADDRESS_IPV6%" == "" (
  ECHO Setting the DNS server on the v6v4 tunnel interface ...
  netsh int ipv6 set dnsserver "%TSP_TUNNEL_INTERFACE%" static %TSP_CLIENT_DNS_ADDRESS_IPV6% none
)

REM Finished setting up the v6v4 tunnel interface.
ECHO V6V4 tunnel configuration successful.
GOTO :EOF


REM -------------------------------------------------------------------------

:V6UDPV4_TUN_PROCEDURE
REM Sets up the V6UDPV4 tunnel, by configuring the gogotun tunnel adapter.

ECHO Setting up a V6UDPV4 tunnel using gogo6's Multi-Virtual Tunnel Adapter.
ECHO Tunnel interface is: "%TSP_TUNNEL_INTERFACE%"

REM Set the IPv6 address on the tunnel interface.
ECHO Setting IPv6 address on v6udpv4 tunnel interface ...
netsh int ipv6 set addr "%TSP_TUNNEL_INTERFACE%" %TSP_CLIENT_ADDRESS_IPV6% %NETSH_PERS% > NUL
IF ERRORLEVEL 1 ( 
  SET ERRNO=30
  GOTO :EOF
)

REM Set the MTU to 1280 on the v6udpv4 tunnel interface.
ECHO Setting MTU to 1280 on v6udpv4 tunnel interface ...
netsh int ipv6 set int "%TSP_TUNNEL_INTERFACE%" MTU=1280 > NUL
IF ERRORLEVEL 1 ( 
  SET ERRNO=31
  GOTO :EOF
)

REM Windows vista hack:
REM If the tunnel interface does not have a configured DNS server in its IPv4
REM stack, resolving of IPv6 addresses will fail.
IF %WIN_VER% NEQ 7 GOTO :SKIP_VISTA_DNS_HACK
ECHO Adding DNS servers to tunnel v6udpv4 tunnel interface ...
IF DEFINED TSP_LOCAL_DNS01 netsh int ipv4 add dnsserver name="%TSP_TUNNEL_INTERFACE%" addr=%TSP_LOCAL_DNS01% > NUL
IF DEFINED TSP_LOCAL_DNS02 netsh int ipv4 add dnsserver name="%TSP_TUNNEL_INTERFACE%" addr=%TSP_LOCAL_DNS02% > NUL
:SKIP_VISTA_DNS_HACK

REM Adding a route to the nexthop (which is the IPv6 address of the server)
ECHO Adding a route to the remote tunnel IPV6 endpoint ...
netsh int ipv6 add route %TSP_SERVER_ADDRESS_IPV6%/128 "%TSP_TUNNEL_INTERFACE%" %NETSH_PERS% > NUL
IF %WIN_VER% NEQ 7 IF ERRORLEVEL 1 ( 
  SET ERRNO=32
  GOTO :EOF
)

REM Route all IPv6 traffic through the v6udpv4 tunnel interface.
ECHO Routing all IPv6 traffic through the v6udpv4 tunnel interface ...
netsh int ipv6 add route ::/0 "%TSP_TUNNEL_INTERFACE%" publish=yes %NETSH_PERS% > NUL
IF %WIN_VER% NEQ 7 IF ERRORLEVEL 1 ( 
  SET ERRNO=33
  GOTO :EOF
)

REM Set the DNS server on the v6udpv4 tunnel interface.
IF NOT "%TSP_CLIENT_DNS_ADDRESS_IPV6%" == "" (
  ECHO Setting the DNS server on the v6udpv4 tunnel interface ...
  netsh int ipv6 set dnsserver "%TSP_TUNNEL_INTERFACE%" static %TSP_CLIENT_DNS_ADDRESS_IPV6% none
)

REM Finished setting up the v6v4 tunnel interface.
ECHO V6UDPV4 tunnel configuration successful.
GOTO :EOF


REM -------------------------------------------------------------------------

:V4V6_TUN_PROCEDURE
REM Sets up the V4V6 tunnel, by configuring the gogotun tunnel adapter.

ECHO Setting up a V4V6 tunnel using gogo6's Multi-Virtual Tunnel Adapter.
ECHO Tunnel interface is: "%TSP_TUNNEL_INTERFACE%"


REM Set the IPv4 address on the tunnel interface.
ECHO Setting IPv4 address on v4v6 tunnel interface ...
netsh int ip set addr "%TSP_TUNNEL_INTERFACE%" static %TSP_CLIENT_ADDRESS_IPV4% 255.255.255.0 > NUL
IF ERRORLEVEL 1 ( 
  SET ERRNO=40
  GOTO :EOF
)

REM Adding a route to the nexthop (which is the IPv4 address of the server)
ECHO Adding a route to the remote tunnel IPv4 endpoint ...
IF %WIN_VER% EQU 7 (
  netsh int ip add route %TSP_SERVER_ADDRESS_IPV4%/32 "%TSP_TUNNEL_INTERFACE%" %TSP_CLIENT_ADDRESS_IPV4% %NETSH_PERS% > NUL
) ELSE (
  route add %TSP_SERVER_ADDRESS_IPV4% mask 255.255.255.255 %TSP_CLIENT_ADDRESS_IPV4%
)
IF ERRORLEVEL 1 ( 
  SET ERRNO=41
  GOTO :EOF
)

REM Route all IPv4 traffic through the v4v6 tunnel interface.
ECHO Routing all IPv4 traffic through the v4v6 tunnel interface ...
IF %WIN_VER% EQU 7 (
  netsh int ip add route 0.0.0.0/0 "%TSP_TUNNEL_INTERFACE%" %TSP_SERVER_ADDRESS_IPV4% publish=yes %NETSH_PERS% > NUL
) ELSE (
  REM Wait 5 secs, then put route. Required by Win2003srvSP2.
  ping -n 5 ::1 > NUL
  route add 0.0.0.0 mask 0.0.0.0 %TSP_SERVER_ADDRESS_IPV4%
)
IF ERRORLEVEL 1 ( 
  SET ERRNO=42
  GOTO :EOF
)

REM Set interface MTU to 1240 for Windows Vista, else it fails transferring big packets.
IF %WIN_VER% EQU 7 (
  ECHO Setting interface MTU to 1240.
  netsh int ip set int "%TSP_TUNNEL_INTERFACE%" mtu=1240 %NETSH_PERS% > NUL

  IF ERRORLEVEL 1 ( 
    SET ERRNO=43
    GOTO :EOF
  )
)

REM Finished setting up the v4v6 tunnel interface.
ECHO V4V6 tunnel configuration successful.
GOTO :EOF


REM -------------------------------------------------------------------------

:ROUTER_CONFIG_PROCEDURE
REM Sets the local computer to send router advertisements on a local 
REM interface.

ECHO Configuring local computer to act as a router.
ECHO Routing advertisements will be published on interface "%TSP_HOME_INTERFACE%".

REM Adding first address of prefix to the publishing interface.
ECHO Adding first address of prefix to the publishing interface ...
netsh int ipv6 set addr "%TSP_HOME_INTERFACE%" %TSP_PREFIX%::1 %NETSH_PERS% > NUL
IF ERRORLEVEL 1 ( 
  SET ERRNO=50
  GOTO :EOF
)

REM Enable forwarding on tunnel interface.
ECHO Enabling forwarding on tunnel interface ...
netsh int ipv6 set int "%TSP_TUNNEL_INTERFACE%" forwarding=enabled > NUL
IF ERRORLEVEL 1 ( 
  SET ERRNO=51
  GOTO :EOF
)

REM Enable forwarding and router advertisements on the publishing interface.
ECHO Enabling forwarding and router advertisement on the publishing interface ...
netsh int ipv6 set int "%TSP_HOME_INTERFACE%" forwarding=enabled advertise=enabled > NUL
IF ERRORLEVEL 1 ( 
  SET ERRNO=52
  GOTO :EOF
)

REM Route the first /64 of the prefix on the tunnel interface.
ECHO Routing first /64 of the prefix on the publishing interface ...
netsh int ipv6 add route %TSP_PREFIX%::/64 "%TSP_HOME_INTERFACE%" siteprefixlength=64 publish=yes %NETSH_PERS% > NUL
IF %WIN_VER% NEQ 7 IF ERRORLEVEL 1 ( 
  SET ERRNO=53
  GOTO :EOF
)

REM Finished router configuration.
ECHO Router configuration successful.
GOTO :EOF


REM -------------------------------------------------------------------------

:OP_TEARDOWN_TUN_PROCEDURE
REM This is the execution path to deconfigure an existing tunnel.
REM No validation is done here. We try to delete it the best we can.


REM Destroy & create a netsh command script to deconfigure tunnel.
ECHO. > %NETSH_COMMAND_SCRIPT%

REM Set up the tunnel, with the negotiated parameters.
IF /i %TSP_TUNNEL_MODE% == v6v4     CALL :V6V4_DECONFIGURE_PROCEDURE >> %NETSH_COMMAND_SCRIPT%
IF /i %TSP_TUNNEL_MODE% == v6udpv4  CALL :V6UV4_DECONFIGURE_PROCEDURE >> %NETSH_COMMAND_SCRIPT%
IF /i %TSP_TUNNEL_MODE% == v4v6     CALL :V4V6_DECONFIGURE_PROCEDURE >> %NETSH_COMMAND_SCRIPT%

REM Perform router deconfiguration, if host type is router.
IF /i %TSP_HOST_TYPE% == router     CALL :RT_DECONFIGURE_PROCEDURE  >> %NETSH_COMMAND_SCRIPT%

REM Run the netsh command script.
netsh -f %NETSH_COMMAND_SCRIPT%

REM Cleanup.
del %NETSH_COMMAND_SCRIPT%


REM Finished tunnel deconfiguration.
GOTO :EOF


REM -------------------------------------------------------------------------

:V6V4_DECONFIGURE_PROCEDURE
REM This procedure deconfigures and deletes the interface for V6V4.
REM Also deletes relevant routes.

REM Override tunnel interface.
SET TSP_TUNNEL_INTERFACE=%V6V4_STATIC_INTERFACE%

REM Context.
ECHO interface ipv6

REM Delete DNS Server.
IF NOT "%TSP_CLIENT_DNS_ADDRESS_IPV6%" == "" (
  ECHO dele dnsserver "%TSP_TUNNEL_INTERFACE%" %TSP_CLIENT_DNS_ADDRESS_IPV6%
)

REM Delete IPv6 address assigned.
ECHO dele addr inter="%TSP_TUNNEL_INTERFACE%" addr=%TSP_CLIENT_ADDRESS_IPV6%

REM Delete route assigned.
ECHO dele route prefix=::/0 inter="%TSP_TUNNEL_INTERFACE%"

REM Delete interface.
ECHO dele inter inter="%V6V4_STATIC_INTERFACE%"

GOTO :EOF


REM -------------------------------------------------------------------------

:V6UV4_DECONFIGURE_PROCEDURE
REM This procedure deconfigures the interface for V6UDPV4.
REM Also deletes relevant routes.

REM Context.
ECHO interface ipv6

REM Delete DNS Server.
IF NOT "%TSP_CLIENT_DNS_ADDRESS_IPV6%" == "" (
  ECHO dele dnsserver "%TSP_TUNNEL_INTERFACE%" %TSP_CLIENT_DNS_ADDRESS_IPV6%
)

REM Delete IPv6 address assigned.
ECHO dele addr inter="%TSP_TUNNEL_INTERFACE%" addr=%TSP_CLIENT_ADDRESS_IPV6%

REM Delete route to nexthop.
ECHO dele route prefix=%TSP_SERVER_ADDRESS_IPV6%/128 inter="%TSP_TUNNEL_INTERFACE%"

REM Delete route assigned.
ECHO dele route prefix=::/0 inter="%TSP_TUNNEL_INTERFACE%"

GOTO :EOF


REM -------------------------------------------------------------------------

:V4V6_DECONFIGURE_PROCEDURE
REM This procedure deconfigures the interface for V4V6.
REM Also deletes relevant routes.

REM Context.
ECHO interface ip

REM Delete assigned IPv4 address.
ECHO dele addr "%TSP_TUNNEL_INTERFACE%" addr=%TSP_CLIENT_ADDRESS_IPV4% gateway=all

REM Delete next-hop route.
IF %WIN_VER% EQU 7 (
  ECHO dele route prefix=%TSP_SERVER_ADDRESS_IPV4%/32 inter="%TSP_TUNNEL_INTERFACE%"
) ELSE (
  route delete %TSP_SERVER_ADDRESS_IPV4% mask 255.255.255.255 %TSP_CLIENT_ADDRESS_IPV4%
)

REM Delete assigned route.
IF %WIN_VER% EQU 7 (
  ECHO dele route prefix=0.0.0.0/0 inter="%TSP_TUNNEL_INTERFACE%"
) ELSE (
  route delete 0.0.0.0 mask 0.0.0.0 %TSP_SERVER_ADDRESS_IPV4%
)

GOTO :EOF


REM -------------------------------------------------------------------------

:RT_DECONFIGURE_PROCEDURE

REM Context.

REM Delete assigned IPv6 address.
ECHO dele addr inter="%TSP_HOME_INTERFACE%" addr=%TSP_PREFIX%::1

REM Disable forwarding on tunnel interface.
ECHO set int inter="%TSP_TUNNEL_INTERFACE%" forwarding=disabled

REM Disable forwarding and router advertisements on the publishing interface.
ECHO set int inter="%TSP_HOME_INTERFACE%" forwarding=disabled advertise=disabled

REM Route the first /64 of the prefix on the tunnel interface.
ECHO dele route prefix=%TSP_PREFIX%::/64 inter="%TSP_HOME_INTERFACE%"

GOTO :EOF


REM -------------------------------------------------------------------------

:PROC_ERR
REM Error procedure. This prints the error message relative to ERRNO and 
REM exits script with the error code.

REM Check if ERRNO is still set to 0. This means that the script was run from
REM outside the gogoCLIENT. Do not call 'Exit' and end script.
IF %ERRNO% EQU 0 (
  ECHO ERROR: This script is meant to be executed inside the gogoCLIENT.
  ECHO Script execution terminated.
  GOTO :END
)

REM Print an error message, relative to the error number previously set.
IF %ERRNO% EQU 10 ECHO ERROR: Failed to identify your Windows version.                                &&  GOTO :END_ERR
IF %ERRNO% EQU 11 ECHO ERROR: This version of windows is not supported by this script.                &&  GOTO :END_ERR
IF %ERRNO% EQU 12 ECHO ERROR: Failed to detect IPv6 connectivity on local computer.                   &&  GOTO :END_ERR
IF %ERRNO% EQU 20 ECHO ERROR: Failed creation of v6v4tunnel.                                          &&  GOTO :END_ERR
IF %ERRNO% EQU 21 ECHO ERROR: Failed addition of local IPv6 address to tunnel interface.              &&  GOTO :END_ERR
IF %ERRNO% EQU 22 ECHO ERROR: Failed adding default route to tunnel interface.                        &&  GOTO :END_ERR
IF %ERRNO% EQU 23 ECHO ERROR: Failed setting MTU on tunnel interface.                                 &&  GOTO :END_ERR
IF %ERRNO% EQU 30 ECHO ERROR: Failed assigning local IPv6 address to tunnel interface.                &&  GOTO :END_ERR
IF %ERRNO% EQU 31 ECHO ERROR: Failed setting MTU on tunnel interface.                                 &&  GOTO :END_ERR
IF %ERRNO% EQU 32 ECHO ERROR: Failed adding a route to the remote tunnel endpoint.                    &&  GOTO :END_ERR
IF %ERRNO% EQU 33 ECHO ERROR: Failed adding default route to tunnel interface.                        &&  GOTO :END_ERR
IF %ERRNO% EQU 40 ECHO ERROR: Failed assigning local IPv4 address to tunnel interface.                &&  GOTO :END_ERR
IF %ERRNO% EQU 41 ECHO ERROR: Failed adding a route to the remote tunnel endpoint.                    &&  GOTO :END_ERR
IF %ERRNO% EQU 42 ECHO ERROR: Failed adding default route to tunnel interface.                        &&  GOTO :END_ERR
IF %ERRNO% EQU 43 ECHO ERROR: Failed setting MTU on tunnel interface.                                 &&  GOTO :END_ERR
IF %ERRNO% EQU 50 ECHO ERROR: Failed assigning first prefix address to publishing interface.          &&  GOTO :END_ERR
IF %ERRNO% EQU 51 ECHO ERROR: Failed enabling forwarding on tunnel interface.                         &&  GOTO :END_ERR
IF %ERRNO% EQU 52 ECHO ERROR: Failed enabling forwarding and advertisements on publishing interface.  &&  GOTO :END_ERR
IF %ERRNO% EQU 53 ECHO ERROR: Failed adding a route on the publishing interface.                      &&  GOTO :END_ERR

:END_ERR
ECHO Template script execution failed! Exiting with error code %ERRNO%.
IF /i "%TSP_SCRIPT_ORIGIN%" == "gogoCLIENT" PAUSE
exit %ERRNO%
GOTO :END


REM -------------------------------------------------------------------------

:END_SUCCESS
REM Script terminated without error.

ECHO Template script execution successful.
GOTO :END


REM -------------------------------------------------------------------------

REM Absolute end of script.
:END
