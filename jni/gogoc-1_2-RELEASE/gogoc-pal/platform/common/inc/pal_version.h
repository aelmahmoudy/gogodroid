/*
-----------------------------------------------------------------------------
 $Id: pal_version.h,v 1.1 2009/11/20 16:38:49 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------

  Platform abstraction layer version.

-----------------------------------------------------------------------------
*/
#ifndef __PAL_VERSION_H__
#define __PAL_VERSION_H__


#ifndef PLATFORM
#define PLATFORM "Unknown platform"
#endif

const char* get_pal_version( void );

#endif
