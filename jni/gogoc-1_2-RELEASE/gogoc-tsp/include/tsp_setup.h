/*
-----------------------------------------------------------------------------
 $Id: tsp_setup.h,v 1.1 2009/11/20 16:53:17 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

#ifndef _TSP_SETUP_H_
#define _TSP_SETUP_H_

#include "config.h"
#include "xml_tun.h"

sint32_t            execScript            ( const char *cmd );
gogoc_status         tspSetupInterface     ( tConf *c, tTunnel *t );
gogoc_status         tspTearDownTunnel     ( tConf* pConf, tTunnel* pTunInfo );

#endif
