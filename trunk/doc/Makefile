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
all: doc

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
.PHONY: all doc doxygen tags mostlyclean clean distclean maintainer-clean

doc: doxygen tags

doxygen:
	doxygen $(top_srcdir)/config/Doxyfile

tags:
	cd $(top_srcdir)/doc/tags && $(MAKE) all
	cp -f $(top_srcdir)/doc/tags/*.html $(top_srcdir)/doc/html/

mostlyclean clean:
	$(RM) *~ *.bak *#

distclean: clean

# This command is intended for maintainers to use; it deletes files
# that may need special tools to rebuild.
maintainer-clean: distclean
	cd $(top_srcdir)/doc/tags && $(MAKE) $(MAKECMDGOALS)
	rm -rf $(top_srcdir)/doc/html/*
