# ###########################################################################
# $Id: Makefile,v 1.1 2009/11/20 16:53:36 jasminko Exp $
#
# Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.
#
# For license information refer to CLIENT-LICENSE.TXT
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

OBJS=$(OBJS_DIR)/base64.o \
	$(OBJS_DIR)/cli.o \
	$(OBJS_DIR)/config.o \
	$(OBJS_DIR)/lib.o \
	$(OBJS_DIR)/log.o \
	$(OBJS_DIR)/md5c.o \
	$(OBJS_DIR)/buffer.o \
	$(OBJS_DIR)/bufaux.o \
	$(OBJS_DIR)/version.o \
	$(OBJS_DIR)/dns.o \
	$(OBJS_DIR)/deque.o

HACCESS_OBJ=$(OBJS_DIR)/haccess.o

all: $(OBJS) $(HACCESS_OBJ)
install: all

$(OBJS_DIR)/base64.o:base64.c
	$(CC) $(CFLAGS) -c base64.c -o $(OBJS_DIR)/base64.o

$(OBJS_DIR)/cli.o:cli.c
	$(CC) $(CFLAGS) -c cli.c -o $(OBJS_DIR)/cli.o

$(OBJS_DIR)/config.o:config.c
	$(CC) $(CFLAGS) -c config.c -o $(OBJS_DIR)/config.o

$(OBJS_DIR)/lib.o:lib.c
	$(CC) $(CFLAGS) -c lib.c -o $(OBJS_DIR)/lib.o

$(OBJS_DIR)/log.o:log.c
	$(CC) $(CFLAGS) -c log.c -o $(OBJS_DIR)/log.o

$(OBJS_DIR)/md5c.o:md5c.c
	$(CC) $(CFLAGS) -c md5c.c -o $(OBJS_DIR)/md5c.o

$(OBJS_DIR)/buffer.o:buffer.c
	$(CC) $(CFLAGS) -c buffer.c -o $(OBJS_DIR)/buffer.o

$(OBJS_DIR)/bufaux.o:bufaux.c
	$(CC) $(CFLAGS) -c bufaux.c -o $(OBJS_DIR)/bufaux.o

$(OBJS_DIR)/version.o:version.c
	$(CC) $(CFLAGS) -c version.c -o $(OBJS_DIR)/version.o

$(OBJS_DIR)/dns.o:dns.c
	$(CC) $(CFLAGS) -c dns.c -o $(OBJS_DIR)/dns.o

$(OBJS_DIR)/deque.o:deque.c
	$(CC) $(CFLAGS) -c deque.c -o $(OBJS_DIR)/deque.o


$(OBJS_DIR)/haccess.o:
	@if [ ! -z "$(HACCESS_DEFINES)" ] && [ -f haccess.c ]; then \
		echo $(CC) $(CFLAGS) -c haccess.c -o $(OBJS_DIR)/haccess.o ;\
		$(CC) $(CFLAGS) -c haccess.c -o $(OBJS_DIR)/haccess.o ;\
	fi

clean:
	rm -f $(OBJS)
	rm -f $(HACCESS_OBJ)
