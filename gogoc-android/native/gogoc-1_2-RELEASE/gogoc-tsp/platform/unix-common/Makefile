# ###########################################################################
# $Id: Makefile,v 1.1 2009/11/20 16:53:30 jasminko Exp $
#
# This source code copyright (c) gogo6 Inc. 2002-2007.
#
#  For license information refer to CLIENT-LICENSE.TXT
#
# ###########################################################################
#
# NOTE: This makefile MUST be interpreted by GNU make.

# Include the platform makefile, if it exists.
-include $(PLATFORM_DIR)/$(PLATFORM)/platform.mak

ifdef DEBUG
CFLAGS=-g -Wall $(CC_INC_PATHS) $(PLATFORM_CFLAGS) -DDEBUG
else
CFLAGS=-O2 -Wall $(CC_INC_PATHS) $(PLATFORM_CFLAGS)
endif
CC=gcc

OS_UNAME=os_uname.h
OBJS=$(OBJS_DIR)/unix-main.o

all: $(OBJS)
install: all

$(OS_UNAME):
	@echo > $(OS_UNAME)
	@echo \#define OS_UNAME_INFO \"`uname -a`\" >> $(OS_UNAME)
	@echo >> $(OS_UNAME)

$(OBJS_DIR)/unix-main.o:unix-main.c $(OS_UNAME)
	$(CC) $(CFLAGS) -c unix-main.c -o $(OBJS_DIR)/unix-main.o

clean:
	rm -f $(OBJS) $(OS_UNAME)

