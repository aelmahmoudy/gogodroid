/*
-----------------------------------------------------------------------------
 $Id: cli.h,v 1.1 2009/11/20 16:53:13 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

#ifndef _CLI_H_
#define _CLI_H_

sint32_t            ask                   (char *question, ...);
void                ParseArguments        (sint32_t, char *[], tConf *);

#endif
