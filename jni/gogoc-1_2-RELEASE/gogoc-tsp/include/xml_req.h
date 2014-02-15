/*
-----------------------------------------------------------------------------
 $Id: xml_req.h,v 1.1 2009/11/20 16:53:18 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

#ifndef _XML_REQ_H_
#define _XML_REQ_H_

#include "config.h"

char *              tspBuildCreateRequest ( tConf * );
char *              tspBuildCreateAcknowledge( void );

#endif
