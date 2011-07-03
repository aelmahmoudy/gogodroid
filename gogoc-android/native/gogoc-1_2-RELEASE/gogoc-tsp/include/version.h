/*
-----------------------------------------------------------------------------
 $Id: version.h,v 1.3 2010/04/20 14:37:40 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

#ifndef VERSION_H
#define VERSION_H

// Software option.
#ifdef HACCESS
#define TSP_CLIENT_OPT_SOFT "HACCESS enabled"
#else
#define TSP_CLIENT_OPT_SOFT ""
#endif

// Architecture type (For Windows builds only).
#ifdef WIN32
  #ifdef BUILD_OPT_X64
    #define TSP_CLIENT_OPT_ARCH "64-bit"
  #else
    #define TSP_CLIENT_OPT_ARCH "32-bit"
  #endif
#else
  #define TSP_CLIENT_OPT_ARCH ""
#endif

// Version number.
#define TSP_CLIENT_VERSION "1.2-RELEASE"

// Identification string.
#define IDENTIFICATION "gogoCLIENT v" TSP_CLIENT_VERSION " build " __DATE__ \
        "-" __TIME__ " " TSP_CLIENT_OPT_SOFT " " TSP_CLIENT_OPT_ARCH


// defined in tsp_client.c
extern char *TspProtocolVersionStrings[];
char *              tsp_get_version       ( void );

#endif

