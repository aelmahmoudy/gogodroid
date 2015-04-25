/*
-----------------------------------------------------------------------------
 $Id: net_cksm.c,v 1.1 2009/11/20 16:53:39 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"
#include "net_cksm.h"

/* checksum calculation */
uint16_t in_cksum( uint16_t *addr, sint32_t len )
{
	sint32_t nleft = len;
	sint32_t sum = 0;
	uint16_t *w = addr;
	uint16_t answer = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}
	if (nleft == 1) {
		*(uint8_t *) (&answer) = *(uint8_t *) w;
		sum += answer;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = (uint16_t)(~sum);
	return (answer);
}
