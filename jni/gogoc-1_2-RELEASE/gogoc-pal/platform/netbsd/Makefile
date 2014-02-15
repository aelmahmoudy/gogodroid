# ###########################################################################
#
# $Id: Makefile,v 1.1 2009/11/20 16:38:50 jasminko Exp $
#
#  Copyright (c) 2007 gogo6 Inc. All rights reserved.
#
#  For license information refer to CLIENT-LICENSE.TXT
#
# Description: Makefile for the netbsd platform
#
# Author: Charles Nepveu
#
# Date: August 2007
#
# ###########################################################################
#

INC_DIR:=inc
SRC_DIR:=src
UNIXCOMMON_INC=../unix-common/inc
COMMON_INC=../common/inc

ifdef DEBUG
CFLAGS+=-c -g -I${DEFS_DIR} -I${INC_DIR} -I${UNIXCOMMON_INC} -I${COMMON_INC} -DPLATFORM=\"${PLATFORM}\" -DDEBUG ${EXTRA_CFLAGS}
else
CFLAGS+=-c -O2 -I${DEFS_DIR} -I${INC_DIR} -I${UNIXCOMMON_INC} -I${COMMON_INC} -DPLATFORM=\"${PLATFORM}\" ${EXTRA_CFLAGS}
endif

OBJS:=$(patsubst ${SRC_DIR}/%.c,${OBJS_DIR}/%.o,$(wildcard ${SRC_DIR}/*.c))


.PHONY: platform-obj platform-inc


#
# ###########################################################################
#

platform-obj: ${OBJS}


${OBJS_DIR}/pal_unistd.o: ${SRC_DIR}/pal_unistd.c ${INC_DIR}/pal_unistd.h
	$(CC) ${CFLAGS} -o $@ ${SRC_DIR}/pal_unistd.c


#
# ###########################################################################
#

platform-inc:
	@cp -f $(wildcard ${INC_DIR}/*.*) $(OUT_INC_DIR)


#
# ###########################################################################
