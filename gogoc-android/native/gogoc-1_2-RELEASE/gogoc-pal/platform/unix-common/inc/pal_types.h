/*
-----------------------------------------------------------------------------
 $Id: pal_types.h,v 1.1 2009/11/20 16:38:54 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------

  Platform abstraction layer for various type definitions.

-----------------------------------------------------------------------------
*/
#ifndef __PAL_TYPES_H__
#define __PAL_TYPES_H__


// Definitions
#undef NULL
#define NULL            (0)

/* Causes conflicts on OpenBSD
// Length type definitions
typedef unsigned int    size_t;
typedef signed int      ssize_t;
*/

// Pointer type defintion
typedef unsigned int    ptr_t;

// Integer type definitions
#ifndef ANDROID
typedef unsigned int    uint32_t;
#endif
typedef signed int      sint32_t;
#ifndef ANDROID
typedef unsigned short  uint16_t;
#endif
typedef signed short    sint16_t;
#ifndef ANDROID
typedef unsigned char   uint8_t;
#endif
typedef signed char     sint8_t;


#endif
