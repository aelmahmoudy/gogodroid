/*
-----------------------------------------------------------------------------
 $Id: pal_string.h,v 1.1 2009/11/20 16:38:53 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------

  Platform abstraction layer for string functions.

-----------------------------------------------------------------------------
*/
#ifndef __PAL_STRING_H__
#define __PAL_STRING_H__


// String API definitions.
#include "pal_string.def"
#include <string.h>


// String functions already available in this platform.
#undef pal_strcpy
#define pal_strcpy strcpy

#undef pal_strncpy
#define pal_strncpy strncpy

#undef pal_strlen
#define pal_strlen (uint32_t)strlen

#undef pal_strcat
#define pal_strcat strcat

#undef pal_strncat
#define pal_strncat strncat

#undef pal_printf
#define pal_printf printf

#undef pal_sprintf
#define pal_sprintf sprintf

#undef pal_snprintf
#define pal_snprintf snprintf

#undef pal_vsnprintf
#define pal_vsnprintf vsnprintf

#undef pal_memcpy
#define pal_memcpy memcpy

#undef pal_strdup
#define pal_strdup strdup

#undef pal_strcasecmp
#define pal_strcasecmp strcasecmp

#undef pal_strncasecmp
#define pal_strncasecmp strncasecmp

#endif
