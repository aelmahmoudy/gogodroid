# ###########################################################################
#
# $Id: Makefile,v 1.1 2009/11/20 16:30:23 jasminko Exp $
#
#  Copyright (c) 2007 gogo6 Inc. All rights reserved.
#
#  For license information refer to CLIENT-LICENSE.TXT
#
# Author: Charles Nepveu
#
# ###########################################################################
#

OBJ_DIR=objs
BIN_DIR=bin
LIB_DIR=lib
INC_DIR=gogocconfig
TARGET=$(LIB_DIR)/libgogocconfig.a

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
LDFLAGS=-g -L$(LIB_DIR) -L$(GOGOCPALLIB_DIR) -lgogocconfig -l$(GOGOCPALLIB) $(EXTRA_LDFLAGS)
else
CXXFLAGS=-O2 -I. -I$(GOGOCPALINC_DIR) -I$(GOGOCPALDEFS_DIR) -Wall -D_REENTRANT -DNDEBUG $(HACCESS_DEFINES) $(EXTRA_CXXFLAGS)
CFLAGS=-O2 -I. -I$(GOGOCPALINC_DIR) -I$(GOGOCPALDEFS_DIR) -Wall -D_REENTRANT -DNDEBUG $(HACCESS_DEFINES) $(EXTRA_CFLAGS)
LDFLAGS=-O2 -L$(LIB_DIR) -L$(GOGOCPALLIB_DIR) -lgogocconfig -l$(GOGOCPALLIB) $(EXTRA_LDFLAGS)
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

OBJS=$(OBJ_DIR)/namevalueparser.o \
	$(OBJ_DIR)/config.o \
	$(OBJ_DIR)/namevalueconfig.o \
	$(OBJ_DIR)/gogocvalidation.o \
	$(OBJ_DIR)/gogocconfig.o \
	$(OBJ_DIR)/gogoc_c_wrapper.o \
	$(OBJ_DIR)/gogocuistrings.o \
	$(OBJ_DIR)/haccess_devmap_c_wrap.o \
	$(OBJ_DIR)/haccessdevicemappingconfig.o


$(OBJ_DIR)/namevalueparser.o: src/namevalueparser.cc $(INC_DIR)/namevalueparser.h
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/namevalueparser.o src/namevalueparser.cc

$(OBJ_DIR)/config.o: src/config.cc $(INC_DIR)/config.h
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/config.o src/config.cc

$(OBJ_DIR)/namevalueconfig.o: src/namevalueconfig.cc $(INC_DIR)/namevalueconfig.h
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/namevalueconfig.o src/namevalueconfig.cc

$(OBJ_DIR)/gogocvalidation.o: src/gogocvalidation.cc $(INC_DIR)/gogocvalidation.h
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/gogocvalidation.o src/gogocvalidation.cc

$(OBJ_DIR)/gogocconfig.o: src/gogocconfig.cc $(INC_DIR)/gogocconfig.h
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/gogocconfig.o src/gogocconfig.cc

$(OBJ_DIR)/gogoc_c_wrapper.o: src/gogoc_c_wrapper.cc $(INC_DIR)/gogoc_c_wrapper.h
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/gogoc_c_wrapper.o src/gogoc_c_wrapper.cc

$(OBJ_DIR)/gogocuistrings.o: src/gogocuistrings.c $(INC_DIR)/gogocuistrings.h
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)/gogocuistrings.o src/gogocuistrings.c

$(OBJ_DIR)/test_namevalue.o: src/test_namevalue.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/test_namevalue.o src/test_namevalue.cc

$(OBJ_DIR)/test_c_wrapper.o: src/test_c_wrapper.c
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)/test_c_wrapper.o src/test_c_wrapper.c

$(OBJ_DIR)/haccess_devmap_c_wrap.o: src/haccess_devmap_c_wrap.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/haccess_devmap_c_wrap.o src/haccess_devmap_c_wrap.cc

$(OBJ_DIR)/haccessdevicemappingconfig.o: src/haccessdevicemappingconfig.cc
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/haccessdevicemappingconfig.o src/haccessdevicemappingconfig.cc

test_targets: $(TARGET) $(OBJ_DIR)/test_namevalue.o $(OBJ_DIR)/test_c_wrapper.o $(BIN_DIR)
	$(LD) -o $(BIN_DIR)/test_namevalue $(OBJ_DIR)/test_namevalue.o $(LDFLAGS)
	$(LD) -o $(BIN_DIR)/test_c_wrapper $(OBJ_DIR)/test_c_wrapper.o $(LDFLAGS)
	

$(TARGET): $(OBJ_DIR) $(OBJS) $(LIB_DIR)
	$(AR) cru $(TARGET) $(OBJS)
	$(RANLIB) $(TARGET)

clean:
	@echo -n "Cleaning up workspace... "
	@rm -rf $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR)
	@echo "done."
