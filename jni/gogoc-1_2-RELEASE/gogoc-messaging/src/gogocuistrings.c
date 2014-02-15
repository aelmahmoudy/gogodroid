/* *********************************************************************** */
/* $Id: gogocuistrings.c,v 1.1 2009/11/20 16:34:55 jasminko Exp $            */
/*                                                                         */
/* Copyright (c) 2007 gogo6 Inc. All rights reserved.                     */
/*                                                                         */
/*   LICENSE NOTICE: You may use and modify this source code only if you   */
/*   have executed a valid license agreement with gogo6 Inc. granting     */
/*   you the right to do so, the said license agreement governing such     */
/*   use and modifications.   Copyright or other intellectual property     */
/*   notices are not to be removed from the source code.                   */
/*                                                                         */
/* Description:                                                            */
/*   Offers default UI string for errors and other.                        */
/*                                                                         */
/* You may translate the strings herein as you wish.                       */
/*                                                                         */
/* Author: Charles Nepveu                                                  */
/*                                                                         */
/* Creation Date: November 2006                                            */
/* _______________________________________________________________________ */
/* *********************************************************************** */
#include <gogocmessaging/gogocuistrings.h>


/* Struct containing string IDs with the related string.                   */
typedef struct { error_t _id; const char* _str; } tgogocUIStrings;


static const tgogocUIStrings gogocUIStrings[] = {

  { GOGOCM_UIS__NOERROR, "SUCCESS" },    // Should never log this, but...

  /* PIPE ERRORS */
  { GOGOCM_UIS_WRITEPIPEFAILED,
    "Failed writing on the named pipe." },
  { GOGOCM_UIS_PEEKPIPEFAILED,
    "Failed \"peeking\" IO status on named pipe." },
  { GOGOCM_UIS_READPIPEFAILED,
    "Failed reading on the named pipe." },
  { GOGOCM_UIS_PIPESERVERALRDUP,
    "Pipe server is already up." },
  { GOGOCM_UIS_FAILCREATESERVERPIPE,
    "Failed creation of pipe server." },
  { GOGOCM_UIS_CLIENTALRDYCONN,
    "Pipe client is already connected." },
  { GOGOCM_UIS_CLIENTCONNFAILED,
    "Pipe client connection failed." },
  { GOGOCM_UIS_PIPESVRDISCFAIL,
    "Pipe server disconnection failed." },
  { GOGOCM_UIS_FAILCREATECLIENTPIPE,
    "Failed creation of client pipe." },
  { GOGOCM_UIS_PIPECLIDISCFAIL,
    "Pipe client disconnection failed." },

  /* IPC LAYER ERRORS */
  { GOGOCM_UIS_BADPACKET,
    "Invalid/erroneous IPC data packet received." },
  { GOGOCM_UIS_IPCDESYNCHRONIZED,
    "IPC communication desynchronized. Need re-initialization." },
  { GOGOCM_UIS_PACKETSNOTORDERED,
    "ERROR, IPC sequential packet number is not ordered." },
  { GOGOCM_UIS_READBUFFERTOOSMALL,
    "IPC layer internal buffer size too small to read data packet." },
  { GOGOCM_UIS_SENDBUFFERTOOBIG,
    "User message data is too big to be sent through the IPC." },
  { GOGOCM_UIS_IOWAITTIMEOUT,
    "Failed acquiring IO mutex to perform requested IPC operation." },

  /* MESSAGING LAYER ERRORS */
  { GOGOCM_UIS_MSGPROCDISABLED,
    "Message processing is disabled. Reception of messages is unavailable." },
  { GOGOCM_UIS_MESSAGENOTIMPL,
    "Unknown message received. Processing for that message is not implemented." },
  { GOGOCM_UIS_CWRAPALRDYINIT,
    "C language wrapper for messaging layer is already initialized." },
  { GOGOCM_UIS_CWRAPNOTINIT,
    "C language wrapper for messaging layer is not implemented call initialize_messaging()." },

  /* GATEWAY6 CLIENT ERRORS */
  { GOGOCM_UIS_ERRUNKNOWN,                 
    "Unknown error." },
  { GOGOCM_UIS_FAILEDBROKERLISTEXTRACTION, 
    "Failed redirection broker list extraction." },
  { GOGOCM_UIS_ERRCFGDATA, 
    "Configuration data is invalid." },
  { GOGOCM_UIS_ERRMEMORYSTARVATION, 
    "Memory allocation error." },
  { GOGOCM_UIS_ERRSOCKETIO, 
    "Socket I/O error." },
  { GOGOCM_UIS_ERRFAILSOCKETCONNECT, 
    "Socket error, cannot connect." },
  { GOGOCM_UIS_EVNTBROKERREDIRECTION, 
    "A redirection has been received from the Gateway6." },
  { GOGOCM_UIS_ERRBROKERREDIRECTION, 
    "Redirection error." },
  { GOGOCM_UIS_ERRTSPVERSIONERROR, 
    "The Gateway6 is not supporting this TSP protocol version." },
  { GOGOCM_UIS_ERRTSPGENERICERROR, 
    "Generic TSP protocol error." },
  { GOGOCM_UIS_ERRTUNMODENOTAVAILABLE, 
    "The requested tunnel mode is not available on the Gateway6." },
  { GOGOCM_UIS_ERRNOCOMMONAUTHENTICATION, 
    "Authentication method is not supported by the Gateway6." },
  { GOGOCM_UIS_ERRAUTHENTICATIONFAILURE, 
    "Authentication failure." },
  { GOGOCM_UIS_ERRBADTUNNELPARAM, 
    "Bad tunnel parameters received from the Gateway6." },
  { GOGOCM_UIS_ERRINTERFACESETUPFAILED, 
    "Failed to execute tunnel configuration script." },
  { GOGOCM_UIS_ERRKEEPALIVETIMEOUT, 
    "A keepalive timeout occurred." },
  { GOGOCM_UIS_ERRKEEPALIVEERROR, 
    "A keepalive network error occurred." },
  { GOGOCM_UIS_ERRTUNNELIO, 
    "Internal tunnel I/O error." },
  { GOGOCM_UIS_ERRTUNLEASEEXPIRED, 
    "The tunnel lease has expired." },
  { GOGOCM_UIS_ERRHACCESSSETUP, 
    "The HACCESS setup script did not complete successfully." },
  { GOGOCM_UIS_ERRHACCESSEXPOSEDEVICES, 
    "The HACCESS subsystem could not apply the HomeAccess configuration successfully." },
  { GOGOCM_UIS_ERRTSPSERVERTOOBUSY,
    "The Gateway6 is too busy to handle your TSP session. Please retry later." },
  { GOGOCM_UIS_ERRINVALSERVERADDR,
    "The Gateway6 address is invalid." }
};


// --------------------------------------------------------------------------
// Function : get_mui_string
//
// Description:
//   Returns the user interface string specified by the id.
//
// Arguments:
//   id: int [IN], The string ID.
//
// Return values:
//   The UI string.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
const char* get_mui_string( const error_t id )
{
  const unsigned int n = sizeof(gogocUIStrings) / sizeof(gogocUIStrings[0]);
  unsigned int i;

  for(i=0; i<n; i++)
    if(gogocUIStrings[i]._id == id)
      return gogocUIStrings[i]._str;

  return (const char*)0;    // NULL
}
