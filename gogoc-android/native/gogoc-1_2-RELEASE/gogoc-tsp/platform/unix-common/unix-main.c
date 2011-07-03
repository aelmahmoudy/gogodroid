/*
-----------------------------------------------------------------------------
 $Id: unix-main.c,v 1.1 2009/11/20 16:53:30 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2001-2006 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT

-----------------------------------------------------------------------------
*/

#include "platform.h"
#include "gogoc_status.h"

#include <signal.h>

#include "tsp_client.h"
#include "hex_strings.h"
#include "log.h"
#include "os_uname.h"

#ifdef HACCESS
#include "haccess.h"
#endif

extern int indSigHUP; /* Declared in every unix platform tsp_local.c */


/* --------------------------------------------------------------------------
// Signal handler function. KEEP THIS FUNCTION AS SIMPLE AS POSSIBLE.
*/
void signal_handler( int sigraised )
{
  if( sigraised == SIGHUP )
    indSigHUP = 1;
}


// --------------------------------------------------------------------------
// Retrieves OS information and puts it nicely in a string ready for display.
//
// Defined in tsp_client.h
//
void tspGetOSInfo( const size_t len, char* buf )
{
  if( len > 0  &&  buf != NULL )
  {
#ifdef OS_UNAME_INFO
    snprintf( buf, len, "Built on ///%s///", OS_UNAME_INFO );
#else
    snprintf( buf, len, "Built on ///unknown UNIX/BSD/Linux version///" );
#endif
  }
}


// --------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  int rc;
#ifdef HACCESS
  haccess_status status = HACCESS_STATUS_OK;
#endif
  /* Install new signal handler for HUP signal. */
  signal( SIGHUP, &signal_handler );

#ifdef HACCESS
  /* Initialize the HACCESS module. */
  status = haccess_initialize();

  if (status != HACCESS_STATUS_OK) {
    DirectErrorMessage(HACCESS_LOG_PREFIX_ERROR GOGO_STR_HACCESS_ERR_CANT_INIT_MODULE);
    return ERR_HACCESS_INIT;
  }
#endif

#ifdef ANDROID
  writepid();
#endif

  /* entry point */
  rc = tspMain(argc, argv);

#ifdef HACCESS
  /* The HACCESS module destructs (deinitialization). */
  status = haccess_destruct();

  if (status != HACCESS_STATUS_OK)
  {
    DirectErrorMessage(HACCESS_LOG_PREFIX_ERROR GOGO_STR_HACCESS_ERR_CANT_DO_SHUTDOWN);
  }
#endif

  return rc;
}
