# ###########################################################################
#
# $Id: Makefile,v 1.2 2010/03/07 16:38:30 carl Exp $
#
#  Copyright (c) 2007 gogo6 Inc. All rights reserved.
#
#  For license information refer to CLIENT-LICENSE.TXT
#
# Description: GNU Makefile for module library gogoc-pal 
#              (gogoCLIENT Platform Abstraction Layer)
#
# Author: Charles Nepveu
#
# Date: October 2007
#
# ###########################################################################
#

PWD           :=$(shell pwd)
PLATFORM_DIR  :=$(PWD)/platform
OBJS_DIR      :=$(PWD)/objs
DEFS_DIR      :=$(PWD)/defs
OUT_LIB_DIR   :=$(PWD)/out_lib
OUT_INC_DIR   :=$(PWD)/out_inc
LIB_NAME      :=libgogocpal.a
GOGOCPAL_LIB   :=${OUT_LIB_DIR}/${LIB_NAME}
PLATFORM      :=$(shell [ -z "$(platform)" ] && uname | tr "[A-Z]" "[a-z]" || echo "$(platform)" )

SUPPORTED_PLATFORMS=linux netbsd freebsd openbsd darwin sunos gogocpe
CC=gcc


# Export these variables to sub-makes.
export PLATFORM PLATFORM_DIR OBJS_DIR DEFS_DIR OUT_INC_DIR CC DEBUG


#
# ###########################################################################
#
.PHONY: platform-check platform-obj platform-lib platform-inc all clean

all: platform-check platform-obj platform-lib platform-inc

# This makefile target will check the platform.
#
platform-check:
	@for plat in ${SUPPORTED_PLATFORMS} ; do \
		[ "${PLATFORM}" = "$$plat" ] && platform_ok=xxx || platform_ok=$$platform_ok ; \
	done && ([ -z "$$platform_ok" ] && { \
	    echo ; \
	    echo "Error: Target platform <${PLATFORM}> is invalid!"; \
	    echo "Syntax: make platform=<target platform>"; \
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
	} || echo "Setting up PAL for platform ${PLATFORM} ..." ; )


# This makefile target will compile the platform PAL objects.
#
platform-obj:
	@echo "Building PAL objects for platform ${PLATFORM} ..."
	@[ -d ${OBJS_DIR} ]  ||  mkdir -p ${OBJS_DIR}
	@($(MAKE) -C ${PLATFORM_DIR} platform-obj) || exit 1
	@echo ;


# This makefile target will create the platform PAL library.
#
platform-lib:
	@echo "Creating ${LIB_NAME} PAL library for platform ${PLATFORM} ..."
	@[ -d ${OUT_LIB_DIR} ]  ||  mkdir -p ${OUT_LIB_DIR}
	@$(AR) -crusv ${GOGOCPAL_LIB} $(wildcard ${OBJS_DIR}/*)
	@echo ;


# This makefile target will copy the platform includes to out_inc.
#
platform-inc:
	@echo "Copying PAL header files for platform ${PLATFORM} ..."
	@[ -d ${OUT_INC_DIR} ]  ||  mkdir -p ${OUT_INC_DIR}
	@($(MAKE) -C ${PLATFORM_DIR} platform-inc) || exit 1
	@echo ;


#
# ###########################################################################
#

clean:
	@echo ;
	rm -rf ${OBJS_DIR}
	rm -rf ${OUT_INC_DIR}
	rm -rf ${OUT_LIB_DIR}
	@echo "Clean complete"
	@echo ;

#
# ###########################################################################
