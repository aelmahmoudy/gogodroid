/*
-----------------------------------------------------------------------------
 $Id: pal_unistd.h,v 1.1 2009/11/20 16:38:50 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------

  Platform abstraction layer for unistd.

-----------------------------------------------------------------------------
*/
#ifndef __PAL_UNISTD_H__
#define __PAL_UNISTD_H__

// unistd API definitions.
#include "pal_unistd.def"
#include <unistd.h>


// unistd functions already available in this platform.
#undef pal_getcwd
#define pal_getcwd getcwd

#undef pal_chdir
#define pal_chdir chdir

#undef pal_unlink
#define pal_unlink unlink

// unistd functions that need coding.
#undef pal_sleep
void                pal_sleep             ( uint32_t );


#endif
