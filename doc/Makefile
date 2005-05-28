# ***************************************************** -*- Makefile -*-
#
# File:      Makefile
# Version:   $Rev$
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   10-Dec-03, ahu: created
#
# Description:
#  Simple Makefile to build the doxygen documentation
#
# Restrictions:
#  Requires GNU make.
#

# Default make target
all: doc tags

# Include system configuration
top_srcdir = ..
include $(top_srcdir)/config/config.mk

# **********************************************************************
# ======================================================================
# **********************************************************************

# Initialisations
SHELL = /bin/sh

.SUFFIXES:

# **********************************************************************
# Targets
.PHONY: all doc tags mostlyclean clean distclean maintainer-clean

doc: 
	doxygen $(top_srcdir)/config/Doxyfile

tags:
	cd tags && $(MAKE) all
	cp -f tags/tags-*.html .

mostlyclean clean distclean:
	$(RM) *~ *.bak *#	
	$(RM) tags-*.html
	cd tags && $(MAKE) $(MAKECMDGOALS)

# This command is intended for maintainers to use; it deletes files
# that may need special tools to rebuild.
maintainer-clean: distclean
	rm -rf $(top_srcdir)/doc/html/*
	cd tags && $(MAKE) distclean
