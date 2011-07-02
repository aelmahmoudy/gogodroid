/*
-----------------------------------------------------------------------------
 $Id: pal_version.c,v 1.1 2009/11/20 16:38:49 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------

  Platform abstraction layer version info.

-----------------------------------------------------------------------------
*/
#include "pal_version.h"

#define PAL_VER_STRING "gogoCLIENT PAL for " PLATFORM " built on " __DATE__

/*
  Returns the pal version.
*/
const char* get_pal_version( void )
{
  return PAL_VER_STRING;
}
