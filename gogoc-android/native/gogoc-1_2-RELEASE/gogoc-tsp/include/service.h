/*
-----------------------------------------------------------------------------
 $Id: service.h,v 1.1 2009/11/20 16:53:16 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2001-2005 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

#ifndef _SERVICE_H_
#define _SERVICE_H_

BOOL                service_init          ( void );
BOOL                service_create        ( TCHAR *name );
BOOL                service_delete        ( TCHAR *name );
void                service_parse_cli     ( int argc, TCHAR *argv[] );
void                service_main          ( int argc, TCHAR *argv[] );

#endif
