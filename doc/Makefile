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
.PHONY: all doc mostlyclean clean distclean maintainer-clean

doc:
	doxygen $(top_srcdir)/config/Doxyfile

mostlyclean clean distclean:
	$(RM) *~ *.bak *#	

# This command is intended for maintainers to use; it deletes files
# that may need special tools to rebuild.
maintainer-clean: distclean
	rm -rf $(top_srcdir)/doc/html/*
