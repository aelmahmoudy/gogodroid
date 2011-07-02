/*
-----------------------------------------------------------------------------
 $Id: pal_stdlib.h,v 1.1 2009/11/20 16:38:53 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------

  Platform abstraction layer stdlib definitions.

-----------------------------------------------------------------------------
*/
#ifndef __PAL_STDLIB_H__
#define __PAL_STDLIB_H__


// stdlib API definitions.
#include "pal_stdlib.def"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>


// Stdlib functions already available in this platform.
#undef pal_free
#define pal_free free

#undef pal_malloc
#define pal_malloc malloc

#undef pal_putenv
#define pal_putenv putenv

#undef pal_srandom
#define pal_srandom srand

#undef pal_random
#define pal_random rand

#undef pal_system
#define pal_system system

#endif
