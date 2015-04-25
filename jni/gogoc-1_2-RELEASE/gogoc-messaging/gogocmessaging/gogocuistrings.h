/* *********************************************************************** */
/* $Id: gogocuistrings.h,v 1.1 2009/11/20 16:34:52 jasminko Exp $           */
/*                                                                         */
/* Copyright (c) 2007 gogo6 Inc. All rights reserved.                     */
/*                                                                         */
/*   For license information refer to CLIENT-LICENSE.TXT                   */
/*                                                                         */
/* Description:                                                            */
/*   Contains the user interface(UI) strings of the Gateway6 Configuration */
/*   subsystem.                                                            */
/*                                                                         */
/* Author: Charles Nepveu                                                  */
/*                                                                         */
/* Creation Date: November 2006                                            */
/* _______________________________________________________________________ */
/* *********************************************************************** */
#ifndef __gogocmessaging_gogocuistrings_h__
#define __gogocmessaging_gogocuistrings_h__


#ifndef ERRORT_DEFINED
#define ERRORT_DEFINED
typedef signed int error_t;
#endif


/* ----------------------------------------------------------------------- */
/* gogoCLIENT User Interface string ID definitions.                   */
/* ----------------------------------------------------------------------- */
#define GOGOCM_UIS__NOERROR                          (error_t)0x00000000

#define GOGOCM_UIS_WRITEPIPEFAILED                   (error_t)0x00000001
#define GOGOCM_UIS_PEEKPIPEFAILED                    (error_t)0x00000002
#define GOGOCM_UIS_READPIPEFAILED                    (error_t)0x00000003
#define GOGOCM_UIS_PIPESERVERALRDUP                  (error_t)0x00000004
#define GOGOCM_UIS_FAILCREATESERVERPIPE              (error_t)0x00000005
#define GOGOCM_UIS_CLIENTALRDYCONN                   (error_t)0x00000006
#define GOGOCM_UIS_CLIENTCONNFAILED                  (error_t)0x00000007
#define GOGOCM_UIS_PIPESVRDISCFAIL                   (error_t)0x00000008
#define GOGOCM_UIS_FAILCREATECLIENTPIPE              (error_t)0x00000009
#define GOGOCM_UIS_PIPECLIDISCFAIL                   (error_t)0x0000000A

#define GOGOCM_UIS_BADPACKET                         (error_t)0x0000000B
#define GOGOCM_UIS_IPCDESYNCHRONIZED                 (error_t)0x0000000C
#define GOGOCM_UIS_PACKETSNOTORDERED                 (error_t)0x0000000D
#define GOGOCM_UIS_READBUFFERTOOSMALL                (error_t)0x0000000E
#define GOGOCM_UIS_SENDBUFFERTOOBIG                  (error_t)0x0000000F
#define GOGOCM_UIS_IOWAITTIMEOUT                     (error_t)0x00000010
#define GOGOCM_UIS_MSGPROCDISABLED                   (error_t)0x00000011
#define GOGOCM_UIS_MESSAGENOTIMPL                    (error_t)0x00000012
#define GOGOCM_UIS_CWRAPALRDYINIT                    (error_t)0x00000013
#define GOGOCM_UIS_CWRAPNOTINIT                      (error_t)0x00000014

// gogoCLIENT errors
#define GOGOCM_UIS_ERRUNKNOWN                        (error_t)0x00000015
#define GOGOCM_UIS_FAILEDBROKERLISTEXTRACTION        (error_t)0x00000016
#define GOGOCM_UIS_ERRCFGDATA                        (error_t)0x00000017
#define GOGOCM_UIS_ERRMEMORYSTARVATION               (error_t)0x00000018
#define GOGOCM_UIS_ERRSOCKETIO                       (error_t)0x00000019
#define GOGOCM_UIS_ERRFAILSOCKETCONNECT              (error_t)0x0000001A
#define GOGOCM_UIS_EVNTBROKERREDIRECTION             (error_t)0x0000001B
#define GOGOCM_UIS_ERRBROKERREDIRECTION              (error_t)0x0000001C
#define GOGOCM_UIS_ERRTSPVERSIONERROR                (error_t)0x0000001D
#define GOGOCM_UIS_ERRTSPGENERICERROR                (error_t)0x0000001E
#define GOGOCM_UIS_ERRTUNMODENOTAVAILABLE            (error_t)0x0000001F
#define GOGOCM_UIS_ERRNOCOMMONAUTHENTICATION         (error_t)0x00000020
#define GOGOCM_UIS_ERRAUTHENTICATIONFAILURE          (error_t)0x00000021
#define GOGOCM_UIS_ERRBADTUNNELPARAM                 (error_t)0x00000022
#define GOGOCM_UIS_ERRINTERFACESETUPFAILED           (error_t)0x00000023
#define GOGOCM_UIS_ERRKEEPALIVETIMEOUT               (error_t)0x00000024
#define GOGOCM_UIS_ERRKEEPALIVEERROR                 (error_t)0x00000025
#define GOGOCM_UIS_ERRTUNNELIO                       (error_t)0x00000026
#define GOGOCM_UIS_ERRTUNLEASEEXPIRED                (error_t)0x00000027
#define GOGOCM_UIS_ERRHACCESSSETUP                   (error_t)0x00000028
#define GOGOCM_UIS_ERRHACCESSEXPOSEDEVICES           (error_t)0x00000029
#define GOGOCM_UIS_ERRTSPSERVERTOOBUSY               (error_t)0x0000002A
#define GOGOCM_UIS_ERRINVALSERVERADDR                (error_t)0x0000002B


/* ----------------------------------------------------------------------- */
/* Get string function.                                                    */
/* ----------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C" const char* get_mui_string( const error_t id );
#else
const char* get_mui_string( const error_t id );
#endif

#endif
