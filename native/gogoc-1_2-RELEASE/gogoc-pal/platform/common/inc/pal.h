/*
-----------------------------------------------------------------------------
 $Id: pal.h,v 1.1 2009/11/20 16:38:49 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------

  Platform abstraction layer.

-----------------------------------------------------------------------------
*/
#ifndef __PAL_H__
#define __PAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "pal_types.h"
#include "pal_version.h"

#include "pal_inet.h"
#include "pal_socket.h"
#include "pal_errno.h"
#include "pal_stdarg.h"
#include "pal_stdlib.h"
#include "pal_getopt.h"
#include "pal_string.h"
#include "pal_unistd.h"
#include "pal_process.h"
#include "pal_time.h"
#include "pal_criticalsection.h"
#include "pal_thread.h"
#include "pal_syslog.h"

#ifdef __cplusplus
}
#endif

#endif
