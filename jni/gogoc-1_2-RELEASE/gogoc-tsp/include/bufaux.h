/*
-----------------------------------------------------------------------------
 $Id: bufaux.h,v 1.1 2009/11/20 16:53:13 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

/*  $NetBSD: bufaux.h,v 1.1.1.7 2002/04/22 07:37:19 itojun Exp $  */
/*  $OpenBSD: bufaux.h,v 1.18 2002/04/20 09:14:58 markus Exp $  */

/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */

#ifndef BUFAUX_H
#define BUFAUX_H

#ifndef NO_OPENSSL

#include "buffer.h"
#include <openssl/bn.h>

void                buffer_put_bignum     (Buffer *, BIGNUM *);
void                buffer_get_bignum     (Buffer *, BIGNUM *);
uint32_t            buffer_get_int        (Buffer *);
void                buffer_put_int        (Buffer *, uint32_t);

sint32_t            buffer_get_octet      (Buffer *);
void                buffer_put_octet      (Buffer *, sint32_t);

void *              buffer_get_string     (Buffer *, uint32_t *);
void                buffer_put_string     (Buffer *, const void *, uint32_t);
void                buffer_put_cstring    (Buffer *, const char *);

#endif    // NO_OPENSSL

#endif    // BUFAUX_H
