# ###########################################################################
#
# $Id: platform.mak,v 1.1 2010/03/07 19:39:09 carl Exp $
#
# Copyright (c) 2007 gogo6 Inc. All rights reserved.
#
#  LICENSE NOTICE: You may use and modify this source code only if you
#  have executed a valid license agreement with gogo6 Inc. granting
#  you the right to do so, the said license agreement governing such
#  use and modifications.   Copyright or other intellectual property
#  notices are not to be removed from the source code.
#
#  This file contains platform-specific makefile rules and symbols.
#
# ###########################################################################
#

# Additional platform CFLAGS.
#
PLATFORM_CFLAGS=$(HACCESS_DEFINES) $(EXTRA_CFLAGS)

# Platform default interface names for each tunnel mode.
#
PLATFORM_V6V4=sit1
PLATFORM_V6UDPV4=tun
PLATFORM_V4V6=
