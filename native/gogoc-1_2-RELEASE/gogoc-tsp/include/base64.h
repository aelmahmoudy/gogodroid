/*
-----------------------------------------------------------------------------
 base64.h - Base64 encoding and decoding prototypes.
-----------------------------------------------------------------------------
 $Id: base64.h,v 1.1 2009/11/20 16:53:12 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

sint32_t            base64decode_len      (const char *bufcoded);
sint32_t            base64decode          (char *bufplain, const char *bufcoded);
sint32_t            base64decode_binary   (unsigned char *bufplain, const char *bufcoded);
sint32_t            base64encode_len      (sint32_t len);
sint32_t            base64encode          (char *encoded, const char *string, sint32_t len);
sint32_t            base64encode_binary   (char *encoded, const unsigned char *string, sint32_t len);

