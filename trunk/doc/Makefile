# ***************************************************** -*- Makefile -*-
#
# Copyright (c) 2003, 2004 Andreas Huggel
# All rights reserved. This program and the accompanying materials 
# are made available under the terms of the Common Public License v1.0
# which accompanies this distribution, and is available at
# http://www.ibm.com/developerworks/oss/CPLv1.0.htm
#
# Author(s): Andreas Huggel (ahu)
#
# RCS information
#  $Name:  $
#  $Revision: 1.1 $
#
# Description:
#  Do NOT change this file! All system specific settings and configs
#  go into config.mk.
#
#  This makefile contains (supposedly) generic build rules to build a
#  library and applications. It includes all system specific settings
#  from config.mk. The idea is that configuring and porting the 
#  software to a new platform should only require changes in config.mk.
#
#  Adding new source files or applications: Just add an entry with the
#  name of the new file in the section 'Source files' below.
#
# Restrictions:
#  Requires GNU make.
#

# Default make target
all: doc

# Include system configuration
top_srcdir = ..
include $(top_srcdir)/config.mk

# **********************************************************************
# ======================================================================
# **********************************************************************

# Initialisations
SHELL = /bin/sh

.SUFFIXES:

# **********************************************************************
# Targets
.PHONY: all doc maintainer-clean

doc:
	doxygen $(top_srcdir)/Doxyfile

# This command is intended for maintainers to use; it deletes files
# that may need special tools to rebuild.
maintainer-clean:
	rm -rf $(top_srcdir)/doc/html/*
