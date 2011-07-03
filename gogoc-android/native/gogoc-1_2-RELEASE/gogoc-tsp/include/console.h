/*
-----------------------------------------------------------------------------
 $Id: console.h,v 1.1 2009/11/20 16:53:13 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2001-2006 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#if !defined(WINCE)
sint32_t            enable_console_input  (void);
sint32_t            disable_console_input (void);
#endif

#endif
