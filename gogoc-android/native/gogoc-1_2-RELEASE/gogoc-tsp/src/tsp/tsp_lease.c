/*
-----------------------------------------------------------------------------
 $Id: tsp_lease.c,v 1.1 2009/11/20 16:53:41 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2001-2006 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"


// --------------------------------------------------------------------------
// This function returns the time at which tunnel will be expired.
// The return value should be used with function tspLeaseCheck().
//
long tspLeaseGetExpTime( const long tun_lifetime )
{
  struct timeval tv;

  // get current time.
  pal_gettimeofday(&tv);

  // calculate expiration time.
  return tv.tv_sec + tun_lifetime;
}


/*
 * This function verify if expiration time is reached.
 */
sint32_t tspLeaseCheckExp( const long tun_expiration )
{
  struct timeval tv;

  /* get current time */
  pal_gettimeofday(&tv);

  /* if expired, return 1 */
  if (tv.tv_sec > tun_expiration)
    return 1;

  else return 0;
}
