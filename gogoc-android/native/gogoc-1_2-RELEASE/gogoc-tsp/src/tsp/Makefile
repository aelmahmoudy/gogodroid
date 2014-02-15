# ###########################################################################
# $Id: Makefile,v 1.1 2009/11/20 16:53:40 jasminko Exp $
#
#  Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.
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

OBJS=$(OBJS_DIR)/tsp_auth.o \
	$(OBJS_DIR)/tsp_cap.o \
	$(OBJS_DIR)/tsp_client.o \
	$(OBJS_DIR)/tsp_net.o \
	$(OBJS_DIR)/tsp_setup.o \
	$(OBJS_DIR)/tsp_auth_passdss.o \
	$(OBJS_DIR)/tsp_lease.o \
	$(OBJS_DIR)/tsp_redirect.o \
	$(OBJS_DIR)/tsp_tun_mgt.o

all: $(OBJS)
install: all

$(OBJS_DIR)/tsp_auth.o:tsp_auth.c
	$(CC) $(CFLAGS) -c tsp_auth.c -o $(OBJS_DIR)/tsp_auth.o

$(OBJS_DIR)/tsp_cap.o:tsp_cap.c
	$(CC) $(CFLAGS) -c tsp_cap.c -o $(OBJS_DIR)/tsp_cap.o

$(OBJS_DIR)/tsp_client.o:tsp_client.c
	$(CC) $(CFLAGS) -c tsp_client.c -o $(OBJS_DIR)/tsp_client.o

$(OBJS_DIR)/tsp_net.o:tsp_net.c
	$(CC) $(CFLAGS) -c tsp_net.c -o $(OBJS_DIR)/tsp_net.o

$(OBJS_DIR)/tsp_setup.o:tsp_setup.c
	$(CC) $(CFLAGS) -c tsp_setup.c -o $(OBJS_DIR)/tsp_setup.o

$(OBJS_DIR)/tsp_auth_passdss.o:tsp_auth_passdss.c
	$(CC) $(CFLAGS) -c tsp_auth_passdss.c -o $(OBJS_DIR)/tsp_auth_passdss.o

$(OBJS_DIR)/tsp_lease.o:tsp_lease.c
	$(CC) $(CFLAGS) -c tsp_lease.c -o $(OBJS_DIR)/tsp_lease.o

$(OBJS_DIR)/tsp_redirect.o:tsp_redirect.c
	$(CC) $(CFLAGS) -c tsp_redirect.c -o $(OBJS_DIR)/tsp_redirect.o

$(OBJS_DIR)/tsp_tun_mgt.o:tsp_tun_mgt.c
	$(CC) $(CFLAGS) -c tsp_tun_mgt.c -o $(OBJS_DIR)/tsp_tun_mgt.o

clean:
	rm -f $(OBJS)
