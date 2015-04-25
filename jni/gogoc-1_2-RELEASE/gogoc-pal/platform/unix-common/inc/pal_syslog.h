/*
-----------------------------------------------------------------------------
 $Id: pal_syslog.h,v 1.1 2009/11/20 16:38:53 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------

  Platform abstraction layer for 'syslog'.

-----------------------------------------------------------------------------
*/
#ifndef __PAL_SYSLOG_H__
#define __PAL_SYSLOG_H__


// syslog API definition.
#include "pal_syslog.def"
#include <syslog.h>


// syslog function already available in this platform.
#undef pal_openlog
#define pal_openlog   openlog

#undef pal_syslog
#define pal_syslog    syslog

#undef pal_closelog
#define pal_closelog  closelog

#endif
