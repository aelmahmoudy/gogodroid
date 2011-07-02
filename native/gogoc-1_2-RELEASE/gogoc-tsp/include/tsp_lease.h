/*
-----------------------------------------------------------------------------
 $Id: tsp_lease.h,v 1.1 2009/11/20 16:53:17 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

#ifndef _TSP_LEASE_H_
#define _TSP_LEASE_H_

long                tspLeaseGetExpTime    ( const long tun_lifetime );

sint32_t            tspLeaseCheckExp      ( const long tun_expiration );

#endif
