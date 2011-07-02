#
# $Id: Makefile,v 1.1 2009/11/20 16:34:50 jasminko Exp $
#
#  Copyright (c) 2007 gogo6 Inc. All rights reserved.
#
#  For license information refer to CLIENT-LICENSE.TXT
#
# Author: Charles Nepveu
#


OBJ_DIR=objs
BIN_DIR=bin
LIB_DIR=lib
INC_DIR=gogocmessaging
TARGET=$(LIB_DIR)/libgogocmessaging.a

GOGOCPAL=../gogoc-pal
GOGOCPALINC_DIR=$(GOGOCPAL)/out_inc
GOGOCPALDEFS_DIR=$(GOGOCPAL)/defs
GOGOCPALLIB_DIR=$(GOGOCPAL)/out_lib
GOGOCPALLIB=gogocpal

CC=gcc
CXX=g++
LD=g++
RANLIB=ranlib
AR=ar

ifdef DEBUG
CXXFLAGS=-g -I. -I$(GOGOCPALINC_DIR) -I$(GOGOCPALDEFS_DIR) -Wall -D_REENTRANT -DDEBUG $(HACCESS_DEFINES) $(EXTRA_CXXFLAGS)
CFLAGS=-g -I. -I$(GOGOCPALINC_DIR) -I$(GOGOCPALDEFS_DIR) -Wall -D_REENTRANT -DDEBUG $(HACCESS_DEFINES) $(EXTRA_CFLAGS)
LDFLAGS=-g -L$(LIB_DIR) -L$(GOGOCPALLIB_DIR) -lgogocmessaging -l$(GOGOCPALLIB) $(EXTRA_LDFLAGS)
else
CXXFLAGS=-O2 -I. -I$(GOGOCPALINC_DIR) -I$(GOGOCPALDEFS_DIR) -Wall -D_REENTRANT -DNDEBUG $(HACCESS_DEFINES) $(EXTRA_CXXFLAGS)
CFLAGS=-O2 -I. -I$(GOGOCPALINC_DIR) -I$(GOGOCPALDEFS_DIR) -Wall -D_REENTRANT -DNDEBUG $(HACCESS_DEFINES) $(EXTRA_CFLAGS)
LDFLAGS=-O2 -L$(LIB_DIR) -L$(GOGOCPALLIB_DIR) -lgogocmessaging -l$(GOGOCPALLIB) $(EXTRA_LDFLAGS)
endif


.PHONY: all clean test_targets


#
#
all: $(TARGET) $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

OBJS=$(OBJ_DIR)/ipcserver.o \
	$(OBJ_DIR)/ipcclient.o \
	$(OBJ_DIR)/servent.o \
	$(OBJ_DIR)/message.o \
	$(OBJ_DIR)/messagesender.o \
	$(OBJ_DIR)/communicationsmgr.o \
	$(OBJ_DIR)/clientmsgtranslator.o \
	$(OBJ_DIR)/clientmsgsender.o \
	$(OBJ_DIR)/servermsgtranslator.o \
	$(OBJ_DIR)/servermsgsender.o \
	$(OBJ_DIR)/clientmessengerimpl.o \
	$(OBJ_DIR)/guimessengerimpl.o \
	$(OBJ_DIR)/gogoc_c_wrapper.o \
	$(OBJ_DIR)/gogocuistrings.o


$(OBJ_DIR)/ipcserver.o: src/ipcserver.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/ipcserver.o src/ipcserver.cc

$(OBJ_DIR)/ipcclient.o: src/ipcclient.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/ipcclient.o src/ipcclient.cc

$(OBJ_DIR)/servent.o: src/servent.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/servent.o src/servent.cc

$(OBJ_DIR)/message.o: src/message.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/message.o src/message.cc

$(OBJ_DIR)/messagesender.o: src/messagesender.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/messagesender.o src/messagesender.cc

$(OBJ_DIR)/communicationsmgr.o: src/communicationsmgr.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/communicationsmgr.o src/communicationsmgr.cc

$(OBJ_DIR)/clientmsgtranslator.o: src/clientmsgtranslator.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/clientmsgtranslator.o src/clientmsgtranslator.cc

$(OBJ_DIR)/clientmsgsender.o: src/clientmsgsender.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/clientmsgsender.o src/clientmsgsender.cc

$(OBJ_DIR)/servermsgtranslator.o: src/servermsgtranslator.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/servermsgtranslator.o src/servermsgtranslator.cc

$(OBJ_DIR)/servermsgsender.o: src/servermsgsender.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/servermsgsender.o src/servermsgsender.cc

$(OBJ_DIR)/clientmessengerimpl.o: src/clientmessengerimpl.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/clientmessengerimpl.o src/clientmessengerimpl.cc

$(OBJ_DIR)/guimessengerimpl.o: src/guimessengerimpl.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/guimessengerimpl.o src/guimessengerimpl.cc

$(OBJ_DIR)/gogoc_c_wrapper.o: src/gogoc_c_wrapper.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/gogoc_c_wrapper.o src/gogoc_c_wrapper.cc

$(OBJ_DIR)/gogocuistrings.o: src/gogocuistrings.c
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)/gogocuistrings.o src/gogocuistrings.c

$(OBJ_DIR)/test_ipclayer.o: src/test_ipclayer.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/test_ipclayer.o src/test_ipclayer.cc

$(OBJ_DIR)/test_messaginglayer.o: src/test_messaginglayer.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/test_messaginglayer.o src/test_messaginglayer.cc

$(OBJ_DIR)/test_gogocemulator.o: src/test_gogocemulator.c
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)/test_gogocemulator.o src/test_gogocemulator.c

test_targets: $(TARGET) $(OBJ_DIR)/test_ipclayer.o $(OBJ_DIR)/test_messaginglayer.o $(OBJ_DIR)/test_gogocemulator.o
	$(LD) -o $(BIN_DIR)/test_ipclayer $(OBJ_DIR)/test_ipclayer.o $(LDFLAGS)
	$(LD) -o $(BIN_DIR)/test_messaginglayer $(OBJ_DIR)/test_messaginglayer.o $(LDFLAGS)
	$(LD) -o $(BIN_DIR)/test_gogocemulator $(OBJ_DIR)/test_gogocemulator.o $(LDFLAGS}


$(TARGET): $(OBJ_DIR) $(OBJS) $(LIB_DIR)
	$(AR) cru $(TARGET) $(OBJS)
	$(RANLIB) $(TARGET)

clean:
	@echo -n "Cleaning up workspace... "
	@rm -rf $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR)
	@echo "done."

