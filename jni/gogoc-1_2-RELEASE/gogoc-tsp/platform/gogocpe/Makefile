# ###########################################################################
# $Id: Makefile,v 1.1 2010/03/07 19:39:08 carl Exp $
#
# This source code copyright (c) gogo6 Inc. 2002-2007.
#
#  LICENSE NOTICE: You may use and modify this source code only if you
#  have executed a valid license agreement with gogo6 Inc. granting
#  you the right to do so, the said license agreement governing such
#  use and modifications.   Copyright or other intellectual property
#  notices are not to be removed from the source code.
#
# ###########################################################################
#
# NOTE: This makefile MUST be interpreted by GNU make.

# Include the platform makefile, if it exists.
-include platform.mak

ifdef DEBUG
CFLAGS=-g -Wall $(CC_INC_PATHS) $(PLATFORM_CFLAGS) -D_REENTRANT -DDEBUG
LDFLAGS=-g $(LD_LIB_PATHS) $(LD_LIBRARIES) $(EXTRA_LDFLAGS)
else
CFLAGS=-O2 -Wall $(CC_INC_PATHS) $(PLATFORM_CFLAGS) -D_REENTRANT
LDFLAGS=$(LD_LIB_PATHS) $(LD_LIBRARIES) $(EXTRA_LDFLAGS)
endif

OBJS=$(OBJS_DIR)/tsp_local.o \
	$(OBJS_DIR)/tsp_tun.o \
	$(OBJS_DIR)/haccess-platform.o


all: $(TARGET)
install: all


$(OBJS_DIR)/tsp_local.o:tsp_local.c
	$(CC) $(CFLAGS) -c tsp_local.c -o $(OBJS_DIR)/tsp_local.o

$(OBJS_DIR)/tsp_tun.o:tsp_tun.c
	$(CC) $(CFLAGS) -c tsp_tun.c -o $(OBJS_DIR)/tsp_tun.o

$(OBJS_DIR)/haccess-platform.o:haccess-platform.c
	$(CC) $(CFLAGS) -c haccess-platform.c -o $(OBJS_DIR)/haccess-platform.o

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(wildcard $(OBJS_DIR)/*.o) $(LDFLAGS)
ifdef DEBUG
	cp $(TARGET) $(TARGET).debug
endif

clean:
	rm -f $(OBJS) $(TARGET)
