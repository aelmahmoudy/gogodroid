# ###########################################################################
# $Id: Makefile,v 1.1 2009/11/20 16:53:38 jasminko Exp $
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

OBJS=$(OBJS_DIR)/net.o \
	$(OBJS_DIR)/net_rudp.o \
	$(OBJS_DIR)/net_rudp6.o \
	$(OBJS_DIR)/net_tcp.o \
	$(OBJS_DIR)/net_udp.o \
	$(OBJS_DIR)/net_ka.o \
	$(OBJS_DIR)/net_cksm.o \
	$(OBJS_DIR)/net_tcp6.o \
	$(OBJS_DIR)/net_echo_request.o \
	$(OBJS_DIR)/icmp_echo_engine.o

all: $(OBJS) 
install: all

$(OBJS_DIR)/net.o:net.c
	$(CC) $(CFLAGS) -c net.c -o $(OBJS_DIR)/net.o

$(OBJS_DIR)/net_rudp.o:net_rudp.c
	$(CC) $(CFLAGS) -c net_rudp.c -o $(OBJS_DIR)/net_rudp.o

$(OBJS_DIR)/net_tcp.o:net_tcp.c
	$(CC) $(CFLAGS) -c net_tcp.c -o $(OBJS_DIR)/net_tcp.o

$(OBJS_DIR)/net_udp.o:net_udp.c
	$(CC) $(CFLAGS) -c net_udp.c -o $(OBJS_DIR)/net_udp.o

$(OBJS_DIR)/net_ka.o:net_ka.c
	$(CC) $(CFLAGS) -c net_ka.c -o $(OBJS_DIR)/net_ka.o

$(OBJS_DIR)/net_cksm.o:net_cksm.c
	$(CC) $(CFLAGS) -c net_cksm.c -o $(OBJS_DIR)/net_cksm.o

$(OBJS_DIR)/net_tcp6.o:net_tcp6.c
	$(CC) $(CFLAGS) -c net_tcp6.c -o $(OBJS_DIR)/net_tcp6.o

$(OBJS_DIR)/net_rudp6.o:net_rudp6.c
	$(CC) $(CFLAGS) -c net_rudp6.c -o $(OBJS_DIR)/net_rudp6.o

$(OBJS_DIR)/net_echo_request.o:net_echo_request.c
	$(CC) $(CFLAGS) -c net_echo_request.c -o $(OBJS_DIR)/net_echo_request.o

$(OBJS_DIR)/icmp_echo_engine.o:icmp_echo_engine.c
	$(CC) $(CFLAGS) -c icmp_echo_engine.c -o $(OBJS_DIR)/icmp_echo_engine.o

clean:
	rm -f $(OBJS)
