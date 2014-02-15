/*
----------------------------------------------------------------------
 haccess-platform.h - HACCESS Module Private Platform Definitions
----------------------------------------------------------------------
 $Id: haccess-platform.h,v 1.1 2010/03/07 19:39:09 carl Exp $
----------------------------------------------------------------------
Copyright (c) 2007 gogo6 Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with gogo6 Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
----------------------------------------------------------------------
*/

#ifndef _HACCESS_PLATFORM_H_
#define _HACCESS_PLATFORM_H_


/*
================================================
================================================
               EXTERNAL SYMBOLS
================================================
================================================
*/


/*
================================================
================================================
              PRIVATE CONSTANTS
================================================
================================================
*/

/* External script locations. */
#define _HACCESS_SCRIPT_LOCATION_SETUP                         "/usr/lib/gogoc/haccess/setup/haccess-setup.sh"
#define _HACCESS_SCRIPT_LOCATION_TEARDOWN                      "/usr/lib/gogoc/haccess/teardown/haccess-teardown.sh"
#define _HACCESS_SCRIPT_LOCATION_PROXY                         "/usr/lib/gogoc/haccess/polipo/generators/haccess-polipo-http-proxy-conf.sh"
#define _HACCESS_SCRIPT_LOCATION_SERVICE_CONTROL               "/usr/lib/gogoc/haccess/polipo/control/haccess-polipo-service-control.sh"

/* The name of the device mapping configuration file. */
#define _HACCESS_DEVICE_MAPPING_FILE                           "/var/etc/haccess-device-mapping.conf"
/* The name of the prepared device mapping configuration file. */
/* This is a version of the file that includes only IPv4 addresses, and */
/* that is used to generate the HTTP proxy configuration. */
#define _HACCESS_PREPARED_DEVICE_MAPPING_FILE                  _HACCESS_DEVICE_MAPPING_FILE ".prepared"

/*
================================================
================================================
              PRIVATE MACROS
================================================
================================================
*/


/*
================================================
================================================
                PRIVATE TYPES
================================================
================================================
*/

/* Possible states for the DDNS hiding done */
/* waiting loop. */
typedef enum
{
  _HACCESS_PLATFORM_HIDING_WAIT_LOOP,
  _HACCESS_PLATFORM_HIDING_WAIT_PROCEED,
  _HACCESS_PLATFORM_HIDING_WAIT_TIMEOUT,
  _HACCESS_PLATFORM_HIDING_WAIT_ERROR
} _haccess_platform_event_wait_state;

/*
================================================
================================================
              PRIVATE VARIABLES
================================================
================================================
*/

/*
================================================
================================================
          PRIVATE FUNCTION DECLARATIONS
================================================
================================================
*/

void *
_haccess_platform_ddns_processing_thread(void *arguments);

haccess_status
_haccess_platform_start_ddns_processing_thread(void *arguments);

haccess_status
_haccess_platform_signal_ddns_request_queue_empty();

haccess_status
_haccess_platform_wait_ddns_hiding_done(int hiding_done_timeout);

int
_haccess_platform_module_is_destructing();

haccess_status
_haccess_platform_set_module_destructing(int thread_wait_delay);

haccess_status
_haccess_platform_initialize();

haccess_status
_haccess_platform_destruct();

#endif
