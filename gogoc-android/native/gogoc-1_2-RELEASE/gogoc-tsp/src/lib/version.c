/*
---------------------------------------------------------------------------
 $Id: version.c,v 1.1 2009/11/20 16:53:38 jasminko Exp $
---------------------------------------------------------------------------
  Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
---------------------------------------------------------------------------
*/

#include "platform.h"

#include "version.h"

char *tsp_get_version(void) {
  return IDENTIFICATION;
}
