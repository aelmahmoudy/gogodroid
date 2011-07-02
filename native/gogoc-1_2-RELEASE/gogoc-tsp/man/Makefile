# $Id: Makefile,v 1.1 2009/11/20 16:53:18 jasminko Exp $
#
#  Copyright (c) 2007 gogo6 Inc. All rights reserved.
#
#  For license information refer to CLIENT-LICENSE.TXT
#

all:

install:
	@echo Installing man pages
	@mkdir -p $(INSTALL_MAN)/man5
	@mkdir -p $(INSTALL_MAN)/man8
	@cp man5/gogoc.conf.5 $(INSTALL_MAN)/man5
	@cp man8/gogoc.8 $(INSTALL_MAN)/man8
	@echo "To view man pages, run :"
	@echo "    man -M $(INSTALL_MAN) gogoc"
	@echo "    man -M $(INSTALL_MAN) gogoc.conf"

clean: 
