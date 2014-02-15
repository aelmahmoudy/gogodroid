/* *********************************************************************** */
/* $Id: gogocuistrings.h,v 1.1 2009/11/20 16:30:24 jasminko Exp $            */
/*                                                                         */
/* Copyright (c) 2007 gogo6 Inc. All rights reserved.                     */
/*                                                                         */
/*   For license information refer to CLIENT-LICENSE.TXT                   */
/*                                                                         */
/* Description:                                                            */
/* Contains the user interface(UI) strings of the gogoCLIENT Configuration */
/*   subsystem.                                                            */
/*                                                                         */
/* Author: Charles Nepveu                                                  */
/*                                                                         */
/* Creation Date: November 2006                                            */
/* _______________________________________________________________________ */
/* *********************************************************************** */
#ifndef __gogocconfig_gogocuistrings_h__
#define __gogocconfig_gogocuistrings_h__


#ifndef ERRORT_DEFINED
#define ERRORT_DEFINED
typedef signed int error_t;
#endif


/* ----------------------------------------------------------------------- */
/* gogoCLIENT User Interface string ID definitions.                   */
/* ----------------------------------------------------------------------- */
#define GOGOC_UIS__NOERROR                              (error_t)0x00000000
#define GOGOC_UIS__GROUP_NAMEVALUEPARSER                (error_t)0x00010000
#define GOGOC_UIS__NMP_OPENFAIL                         (error_t)0x00010001
#define GOGOC_UIS__NMP_BADCONFIGFILE                    (error_t)0x00010002
#define GOGOC_UIS__NMP_OPENFAILWRITE                    (error_t)0x00010003
#define GOGOC_UIS__GROUP_GENERICCONFIG                  (error_t)0x00020000
#define GOGOC_UIS__CFG_CANNOTLOADWHENCREATE             (error_t)0x00020001
#define GOGOC_UIS__CFG_CANNOTAPPLYWHENREAD              (error_t)0x00020002
#define GOGOC_UIS__CFG_CANNOTCANCELWHENREAD             (error_t)0x00020003
#define GOGOC_UIS__CFG_CANNOTOVERRIDESAMECONTENTS       (error_t)0x00020004
#define GOGOC_UIS__CFG_CANNOTOVERRIDEWHENREAD           (error_t)0x00020005
#define GOGOC_UIS__GROUP_GOGOCCONFIG                    (error_t)0x00030000
#define GOGOC_UIS__G6C_INVALIDCONF                      (error_t)0x00030001
#define GOGOC_UIS__G6C_FAILLOADDFLTCONF                 (error_t)0x00030002
#define GOGOC_UIS__G6C_SUPPLYPASSWDWHENNOTANON          (error_t)0x00030003
#define GOGOC_UIS__G6C_PROXYCINVALIDMODE                (error_t)0x00030004
#define GOGOC_UIS__G6C_KAINTERVALINVALID                (error_t)0x00030005
#define GOGOC_UIS__G6C_IFTUNV6V4ANDV6UDPV4REQUIRED      (error_t)0x00030006
#define GOGOC_UIS__G6C_IFTUNV6V4REQUIRED                (error_t)0x00030007
#define GOGOC_UIS__G6C_IFTUNV6UDPV4REQUIRED             (error_t)0x00030008
#define GOGOC_UIS__G6C_IFTUNV4V6REQUIRED                (error_t)0x00030009
#define GOGOC_UIS__GROUP_GOGOCVALIDATION                (error_t)0x00040000
#define GOGOC_UIS__G6V_USERIDTOOLONG                    (error_t)0x00040001
#define GOGOC_UIS__G6V_USERIDINVALIDCHRS                (error_t)0x00040002
#define GOGOC_UIS__G6V_PASSWDTOOLONG                    (error_t)0x00040003
#define GOGOC_UIS__G6V_SERVERMUSTBESPEC                 (error_t)0x00040004
#define GOGOC_UIS__G6V_SERVERTOOLONG                    (error_t)0x00040005
#define GOGOC_UIS__G6V_SERVERINVALIDCHRS                (error_t)0x00040006
#define GOGOC_UIS__G6V_HOSTTYPEINVALIDVALUE             (error_t)0x00040007
#define GOGOC_UIS__G6V_PREFIXLENINVALIDVALUE            (error_t)0x00040008
#define GOGOC_UIS__G6V_IFPREFIXINVALIDCHRS              (error_t)0x00040009
#define GOGOC_UIS__G6V_DNSSERVERSTOOLONG                (error_t)0x0004000A
#define GOGOC_UIS__G6V_DNSSERVERSUNRESOLVABLE           (error_t)0x0004000B
#define GOGOC_UIS__G6V_GOGOCDIRDOESNTEXIST              (error_t)0x0004000C
#define GOGOC_UIS__G6V_AUTHMETHODINVALIDVALUE           (error_t)0x0004000D
#define GOGOC_UIS__G6V_AUTORETRYCONNECTINVALIDVALUE     (error_t)0x0004000E
#define GOGOC_UIS__G6V_RETRYDELAYINVALIDVALUE           (error_t)0x0004000F
#define GOGOC_UIS__G6V_KEEPALIVEINVALIDVALUE            (error_t)0x00040010
#define GOGOC_UIS__G6V_KEEPALIVEINTERVINVALID           (error_t)0x00040011
#define GOGOC_UIS__G6V_TUNNELMODEINVALIDVALUE           (error_t)0x00040012
#define GOGOC_UIS__G6V_IFTUNV6V4INVALIDCHRS             (error_t)0x00040013
#define GOGOC_UIS__G6V_IFTUNV6UDPV4INVALIDCHRS          (error_t)0x00040014
#define GOGOC_UIS__G6V_IFTUNV4V6INVALIDCHRS             (error_t)0x00040015
#define GOGOC_UIS__G6V_CLIENTV4INVALIDVALUE             (error_t)0x00040016
#define GOGOC_UIS__G6V_CLIENTV6INVALIDVALUE             (error_t)0x00040017
#define GOGOC_UIS__G6V_TEMPLATEINVALIDVALUE             (error_t)0x00040018
#define GOGOC_UIS__G6V_PROXYCLIENTINVALIDVALUE          (error_t)0x00040019
#define GOGOC_UIS__G6V_BROKERLISTTOOLONG                (error_t)0x0004001A
#define GOGOC_UIS__G6V_BROKERLISTINVALIDCHRS            (error_t)0x0004001B
#define GOGOC_UIS__G6V_LASTSERVTOOLONG                  (error_t)0x0004001C
#define GOGOC_UIS__G6V_LASTSERVINVALIDCHRS              (error_t)0x0004001D
#define GOGOC_UIS__G6V_ALWAYSUSERLASTSERVINVALIDVALUE   (error_t)0x0004001E
#define GOGOC_UIS__G6V_LOGLEVELINVALIDVALUE             (error_t)0x0004001F
#define GOGOC_UIS__G6V_LOGDEVICEINVALIDVALUE            (error_t)0x00040020
#define GOGOC_UIS__G6V_LOGFILENAMETOOLONG               (error_t)0x00040021
#define GOGOC_UIS__G6V_LOGFILENAMEINVALIDCHRS           (error_t)0x00040022
#define GOGOC_UIS__G6V_LOGROTATIONINVALIDVALUE          (error_t)0x00040023
#define GOGOC_UIS__G6V_LOGROTSZINVALIDVALUE             (error_t)0x00040024
#define GOGOC_UIS__G6V_SYSLOGFACILITYINVALIDVALUE       (error_t)0x00040025
#define GOGOC_UIS__G6V_DNSSERVERSINVALIDCHRS            (error_t)0x00040026
#define GOGOC_UIS__G6V_HACCESSPROXYENABLEDINVALIDVALUE  (error_t)0x00040027
#define GOGOC_UIS__G6V_HACCESSWEBENABLEDINVALIDVALUE    (error_t)0x00040028
#define GOGOC_UIS__G6V_HACCESSDOCROOTDOESNTEXIST        (error_t)0x00040029
#define GOGOC_UIS__G6V_HACCESSDOCROOTNOTSPEC            (error_t)0x0004002A
#define GOGOC_UIS__G6V_HACCESSINCOMPV4V6                (error_t)0x0004002B
#define GOGOC_UIS__G6V_PASSWDINVALIDCHRS                (error_t)0x0004002C
#define GOGOC_UIS__G6V_IFPREFIXMUSTBESPEC               (error_t)0x0004002D
#define GOGOC_UIS__G6V_LOGROTDELINVALIDVALUE            (error_t)0x0004002F
#define GOGOC_UIS__G6C_PROXYANDKEEPALIVE                (error_t)0x00040030
#define GOGOC_UIS__G6V_RETRYDELAYMAXINVALIDVALUE        (error_t)0x00040031
#define GOGOC_UIS__G6V_RETRYDELAYGREATERRETRYDELAYMAX   (error_t)0x00040032

/* ----------------------------------------------------------------------- */
/* Get string function.                                                    */
/* ----------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C" const char* get_ui_string( const error_t id );
#else
const char* get_ui_string( const error_t id );
#endif

#endif
