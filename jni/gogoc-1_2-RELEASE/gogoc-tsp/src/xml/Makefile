# ###########################################################################
# $Id: Makefile,v 1.1 2009/11/20 16:53:42 jasminko Exp $
#
# Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.
#
#  For license information refer to CLIENT-LICENSE.TXT
#
# ###########################################################################
#
# NOTE: This makefile MUST be interpreted by GNU make.

# Include the platform makefile, if it exists.
-include $(PLATFORM_DIR)/$(PLATFORM)/platform.mak


ifdef DEBUG
CFLAGS=-g -Wall $(CC_INC_PATHS) $(PLATFORM_CFLAGS) -D_REENTRANT -DDEBUG
else
CFLAGS=-O2 -Wall $(CC_INC_PATHS) $(PLATFORM_CFLAGS) -D_REENTRANT
endif
CC=gcc

OBJS=$(OBJS_DIR)/xmlparse.o \
	$(OBJS_DIR)/xml_req.o \
	$(OBJS_DIR)/xml_tun.o


all: $(OBJS) 
install: all


$(OBJS_DIR)/xmlparse.o:xmlparse.c
	$(CC) $(CFLAGS) -c xmlparse.c -o $(OBJS_DIR)/xmlparse.o $(DEFINES)

$(OBJS_DIR)/xml_req.o:xml_req.c
	$(CC) $(CFLAGS) -c xml_req.c -o $(OBJS_DIR)/xml_req.o $(DEFINES)

$(OBJS_DIR)/xml_tun.o:xml_tun.c
	$(CC) $(CFLAGS) -c xml_tun.c -o $(OBJS_DIR)/xml_tun.o $(DEFINES)

clean:
	rm -f $(OBJS)
