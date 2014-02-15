/*
-----------------------------------------------------------------------------
 $Id: net_ka_winxp.h,v 1.1 2009/11/20 16:53:15 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

extern sint32_t     winxp_use_old_ka;   // Defined in winpc/tsp_local.c

sint32_t            NetKeepaliveInit    ( char *src, char *dst, sint32_t maximal_keepalive, sint32_t family );
void                NetKeepaliveDestroy ( void );
sint32_t            NetKeepaliveDo      ( void );
void                NetKeepaliveGotRead ( void );
void                NetKeepaliveGotWrite( void );
