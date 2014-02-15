# ###########################################################################
#
# $Id: Makefile,v 1.1 2009/11/20 16:53:43 jasminko Exp $
#
#  This source code copyright (c) gogo6 Inc. 2005,2007.
#
#  LICENSE NOTICE: You may use and modify this source code only if you
#  have executed a valid license agreement with gogo6 Inc. granting
#  you the right to do so, the said license agreement governing such
#  use and modifications.   Copyright or other intellectual property
#  notices are not to be removed from the source code.
#
# ###########################################################################
#

all:

install:
	@echo "Installing templates ..."
	@for template_var in README $(PLATFORM).sh; do \
            cp $${template_var} $(INSTALL_TEMPL)/$${template_var}; \
        done

clean: 
