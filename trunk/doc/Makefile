# ************************************************************* -*- Makefile -*-
#
# Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net>
#
# This Makefile is part of the Exiv2 distribution.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.
#
# File:      Makefile
# Version:   $Name:  $ $Revision: 1.2 $
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   10-Dec-03, ahu: created
#
# Description:
#  Simple Makefile to build the doxygen documentation
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
