/*
-----------------------------------------------------------------------------
 $Id: pal_thread.h,v 1.1 2009/11/20 16:38:53 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------

  Platform abstraction layer thread definitions.

-----------------------------------------------------------------------------
*/
#ifndef __PAL_THREAD_H__
#define __PAL_THREAD_H__


#include <pthread.h>
#define PAL_THREAD_CALL
typedef pthread_t           pal_thread_t;
typedef void *              pal_thread_ret_t;
typedef pal_thread_ret_t    (*pal_thread_funct)(void *);


// Thread API definitions.
#include "pal_thread.def"


// Thread functions already available in this platform.
#undef pal_thread_create
#define pal_thread_create(X, Y, Z)  pthread_create(X, NULL, Y, Z)

#undef pal_thread_exit
#define pal_thread_exit             pthread_exit

#undef pal_thread_join
#define pal_thread_join             pthread_join

#endif
