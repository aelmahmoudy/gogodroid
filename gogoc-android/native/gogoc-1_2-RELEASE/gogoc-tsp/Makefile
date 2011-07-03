# ###########################################################################
#
# $Id: Makefile,v 1.2 2010/03/07 19:49:54 carl Exp $
#
#  Copyright (c) 2007 gogo6 Inc. All rights reserved.
#
#  For license information refer to CLIENT-LICENSE.TXT
#
# Description: GNU Makefile for gogoCLIENT application.
#              This application requires the following modules:
#              - gogoc-pal      : Platform Abstraction Layer
#              - gogoc-config   : Configuration Subsystem
#              - gogoc-messaging: Messaging Subsystem
#
# Usage:
#       make [platform=<your platform>] [DEBUG=1] all
#       make [platform=<your platform>] <installdir=/path/to/install> install
#       This makefile will attempt to detect your platform if not supplied.
#
# Author: Charles Nepveu
#
# Date: October 2007
#
# ###########################################################################
#

# NOTE: This makefile MUST be interpreted by GNU make.

PWD           :=$(shell pwd)
PLATFORM_DIR  :=$(PWD)/platform
OBJS_DIR      :=$(PWD)/objs
BIN_DIR       :=$(PWD)/bin
TARGET        :=$(BIN_DIR)/gogoc
GOGOCPAL_DIR   :=$(PWD)/../gogoc-pal
GOGOCPAL_INCDIR:=$(GOGOCPAL_DIR)/out_inc
GOGOCPAL_DEFDIR:=$(GOGOCPAL_DIR)/defs
GOGOCPAL_LIBDIR:=$(GOGOCPAL_DIR)/out_lib
GOGOCCFG_DIR   :=$(PWD)/../gogoc-config
GOGOCCFG_INCDIR:=$(GOGOCCFG_DIR)
GOGOCCFG_LIBDIR:=$(GOGOCCFG_DIR)/lib
GOGOCMSG_DIR   :=$(PWD)/../gogoc-messaging
GOGOCMSG_INCDIR:=$(GOGOCMSG_DIR)
GOGOCMSG_LIBDIR:=$(GOGOCMSG_DIR)/lib

PLATFORM      :=$(shell [ -z "$(platform)" ] && uname | tr "[A-Z]" "[a-z]" || echo "$(platform)" )
SUPPORTED_PLATFORMS=linux netbsd freebsd openbsd darwin sunos gogocpe

INSTALL_DIR   :=$(installdir)
INSTALL_BIN   :=$(INSTALL_DIR)/bin
INSTALL_MAN   :=$(INSTALL_DIR)/man
INSTALL_TEMPL :=$(INSTALL_DIR)/template


SUBDIRS=$(PWD)/src/lib \
	$(PWD)/src/net \
	$(PWD)/src/tsp \
	$(PWD)/src/xml \
	$(PLATFORM_DIR)/unix-common \
	$(PLATFORM_DIR)/$(PLATFORM) \
	$(PWD)/template \
	$(PWD)/conf \
	$(PWD)/man

CC_INC_PATHS=-I$(PLATFORM_DIR)/$(PLATFORM) -I$(PWD)/include -I$(GOGOCPAL_INCDIR) -I$(GOGOCPAL_DEFDIR) -I$(GOGOCCFG_INCDIR) -I$(GOGOCMSG_INCDIR)
LD_LIB_PATHS=-L$(GOGOCPAL_LIBDIR) -L$(GOGOCCFG_LIBDIR) -L$(GOGOCMSG_LIBDIR)
LD_LIBRARIES=-lgogocpal -lgogocconfig -lgogocmessaging

# Export these variables to sub-makes.
export PLATFORM_DIR PLATFORM BIN_DIR OBJS_DIR TARGET DEBUG CC_INC_PATHS LD_LIB_PATHS LD_LIBRARIES INSTALL_DIR INSTALL_BIN INSTALL_MAN INSTALL_TEMPL


#
# ###########################################################################
#
.PHONY: all platform-check check-gogoc-pal check-gogoc-config check-gogoc-messaging build-gogoc check-gogoc-install install clean cleanall

all: platform-check check-gogoc-pal check-gogoc-config check-gogoc-messaging build-gogoc


# This makefile target will check the platform.
#
platform-check:
	@for plat in ${SUPPORTED_PLATFORMS} ; do \
		[ "${PLATFORM}" = "$$plat" ] && platform_ok=xxx || platform_ok=$$platform_ok ; \
	done && ([ -z "$$platform_ok" ] && { \
	    echo ; \
	    echo "Error: Target platform <${PLATFORM}> is invalid!"; \
	    echo "Syntax: make platform=<target platform> all"; \
	    echo ; \
	    echo "    where <target platform> is one of the following:"; \
	    echo "        linux        for Linux."          ; \
	    echo "        freebsd      for FreeBSD."        ; \
	    echo "        openbsd      for OpenBSD."        ; \
	    echo "        netbsd       for NetBSD."         ; \
	    echo "        darwin       for Mac OS X darwin."; \
	    echo "        openwrt      for OpenWRT."        ; \
	    echo "        sunos        for Sun/Solaris."    ; \
	    echo ; \
	    exit 1;\
	} || echo "Building gogoCLIENT for platform ${PLATFORM} ..." ; )


# This makefile target will check and build the gogoCLIENT Platform 
# Abstraction Layer if it is not built.
#
check-gogoc-pal:
	@[ -d ${GOGOCPAL_DIR} ] || { \
	    echo "gogoCLIENT Platform Abstraction Layer module is not found. (${GOGOCPAL_DIR})"; \
	    exit 1 ; \
	}
	@[ -f ${GOGOCPAL_LIBDIR}/libgogocpal.a ] || { \
	    echo "Building gogoc-pal module ..." ; \
	    $(MAKE) -C ${GOGOCPAL_DIR} ; \
	}


# This makefile target will check and build the gogoCLIENT Configuration
# Subsystem if it is not built.
#
check-gogoc-config:
	@[ -d ${GOGOCCFG_DIR} ] || { \
	    echo "gogoCLIENT Configuration Subsystem module is not found. (${GOGOCCFG_DIR})"; \
	    exit 1 ; \
	}
	@[ -f ${GOGOCCFG_LIBDIR}/libgogocconfig.a ] || { \
	    echo "Building gogoc-config module ..." ; \
	    $(MAKE) -C ${GOGOCCFG_DIR} ; \
	}


# This makefile target will check and build the gogoCLIENT Messaging
# Subsystem if it is not built.
#
check-gogoc-messaging:
	@[ -d ${GOGOCMSG_DIR} ] || { \
	    echo "gogoCLIENT Messaging Subsystem module is not found. (${GOGOCMSG_DIR})"; \
	    exit 1 ; \
	}
	@[ -f ${GOGOCMSG_LIBDIR}/libgogocmessaging.a ] || { \
	    echo "Building gogoc-messaging module ..." ; \
	    $(MAKE) -C ${GOGOCMSG_DIR} ; \
	}


# This makefile target will build the gogoCLIENT.
#
build-gogoc:
	mkdir -p $(OBJS_DIR)
	mkdir -p $(BIN_DIR)
	@for dir in ${SUBDIRS}; do \
	    $(MAKE) -C $$dir all || exit 1; \
	done


# This makefile target will install the gogoCLIENT.
#
check-gogoc-install:
	@[ -n "$(INSTALL_DIR)" ] || { \
	    echo ; \
	    echo "Error: You must specify the install directory"; \
	    echo "Syntax: make [platform=<os-type>] installdir=</path/to/install> install"; \
	    echo ; \
	    exit 1;\
	}

install: check-gogoc-install all
	@mkdir -p $(INSTALL_DIR)
	@mkdir -p $(INSTALL_BIN)
	@mkdir -p $(INSTALL_MAN)
	@mkdir -p $(INSTALL_TEMPL)

	@for dir in ${SUBDIRS}; do \
	    $(MAKE) -C $$dir install || exit 1; \
	done

	@cp $(TARGET) $(INSTALL_BIN)
	@cp $(BIN_DIR)/gogoc.conf.sample $(INSTALL_BIN)
	@[ -f $(INSTALL_BIN)/gogoc.conf ] || { \
	    cp $(INSTALL_BIN)/gogoc.conf.sample $(INSTALL_BIN)/gogoc.conf; \
	}


# This makefile target will clean the build tree of the gogoCLIENT.
#
clean:
	@for dir in ${SUBDIRS}; do \
	    $(MAKE) -C $$dir clean || exit 1; \
	done
	rm -rf $(TARGET) $(BIN_DIR)/gogoc.conf.sample $(OBJS_DIR)


# This makefile target will clean the build tree of the gogoCLIENT,
# and the submodules.
#
cleanall: clean
	$(MAKE) -C ${GOGOCMSG_DIR} clean
	$(MAKE) -C ${GOGOCCFG_DIR} clean
	$(MAKE) -C ${GOGOCPAL_DIR} clean
